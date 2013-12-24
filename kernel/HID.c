/*

Nintendont (Kernel) - Playing Gamecubes in Wii mode on a Wii U

Copyright (C) 2013  crediar

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/
#include "HID.h"
#include "ff.h"

#ifndef DEBUG_HID
#define dbgprintf(...)
#else
extern int dbgprintf( const char *fmt, ...);
#endif

struct SickSaxis PS3Controller = {{0}};

static u8 ss_led_pattern[8] = {0x0, 0x02, 0x04, 0x08, 0x10, 0x12, 0x14, 0x18};

s32 HIDHandle = 0;
u32 PS3LedSet = 0;
u32 DeviceID  = 0;
u32 bEndpointAddress = 0;
u32 wMaxPacketSize	 = 0;
u8 *Packet = (u8*)NULL;
controller *CTRL;

req_args *req = (req_args *)NULL;

s32 HIDInit( void )
{
	s32 ret;
	dbgprintf("HIDInit()\n");
	HIDHandle = IOS_Open("/dev/usb/hid", 0 );

	memset32( &PS3Controller, 0, sizeof( struct SickSaxis ) );
	
	char *HIDHeap = (char*)malloca( 0x600, 32 );
	memset32( HIDHeap, 0, 0x600 );

	req	= (req_args*)malloca( sizeof(req_args), 32 );

	CTRL = (controller *)malloca( sizeof(controller), 32 );
	
	ret = IOS_Ioctl( HIDHandle, /*GetDeviceChange*/0, NULL, 0, HIDHeap, 0x600 );
	if( ret < 0 )
	{
		dbgprintf("HID:GetDeviceChange():%d\n", ret );
		return -1;
	}

	DeviceID	= *(vu32*)(HIDHeap+4);
	HIDHandle	= HIDHandle;
			
	dbgprintf("HID:DeviceID:%u\n", DeviceID );
	dbgprintf("HID:VID:%04X PID:%04X\n", *(vu16*)(HIDHeap+0x10), *(vu16*)(HIDHeap+0x12) );
			
	u32 Offset = 8;
		
	u32 DeviceDescLength	= *(vu8*)(HIDHeap+Offset);
	Offset += (DeviceDescLength+3)&(~3);

	u32 ConfigurationLength = *(vu8*)(HIDHeap+Offset);
	Offset += (ConfigurationLength+3)&(~3);
		
	u32 InterfaceDescLength	= *(vu8*)(HIDHeap+Offset);
	Offset += (InterfaceDescLength+3)&(~3);
		
	u32 EndpointDescLengthO	= *(vu8*)(HIDHeap+Offset);
		
	bEndpointAddress = *(vu8*)(HIDHeap+Offset+2);

	if( (bEndpointAddress & 0xF0) != 0x80 )
		Offset += (EndpointDescLengthO+3)&(~3);

	bEndpointAddress = *(vu8*)(HIDHeap+Offset+2);
	wMaxPacketSize	 = *(vu16*)(HIDHeap+Offset+4);
		
	dbgprintf("HID:bEndpointAddress:%02X\n", bEndpointAddress );
	dbgprintf("HID:wMaxPacketSize  :%u\n", wMaxPacketSize );

	if( *(vu16*)(HIDHeap+0x10) == 0x054c && *(vu16*)(HIDHeap+0x12) == 0x0268 )
	{
		dbgprintf("HID:PS3 Dualshock Controller detected\n");
		HIDPS3Init( &PS3Controller );

		HIDPS3SetRumble( 0, 0, 0, 0 );

		Packet = (u8*)malloc(SS_DATA_LEN);
	}

//Load controller config
	FIL f;
	u32 read;

	ret = f_open( &f, "/controller.ini", FA_OPEN_EXISTING|FA_READ);
	if( ret == FR_OK )
	{
		char *Data = (char*)malloc( f.fsize );
		f_read( &f, Data, f.fsize, &read );
		f_close(&f);

		CTRL->VID = ConfigGetValue( Data, "VID", 0 );
		CTRL->PID = ConfigGetValue( Data, "PID", 0 );

		if( *(vu16*)(HIDHeap+0x10) != CTRL->VID || *(vu16*)(HIDHeap+0x12) != CTRL->PID )
		{
			dbgprintf("HID:Config does not match device VID/PID\n");
			dbgprintf("HID:Config VID:%04X PID:%04X\n", CTRL->VID, CTRL->PID );
			return -3;
		}
	
		CTRL->DPAD		= ConfigGetValue( Data, "DPAD", 0 );
		CTRL->Polltype	= ConfigGetValue( Data, "Polltype", 0 );
		CTRL->MultiIn	= ConfigGetValue( Data, "MultiIn", 0 );
		if( CTRL->MultiIn )
		{
			CTRL->MultiInValue= ConfigGetValue( Data, "MultiInValue", 0 );
		
			dbgprintf("HID:MultIn:%u\n", CTRL->MultiIn );
			dbgprintf("HID:MultiInValue:%u\n", CTRL->MultiInValue );
		}

		if( Packet == (u8*)NULL )
		{
			if( CTRL->Polltype )
			{
				Packet = (u8*)malloc(wMaxPacketSize);
			} else if( CTRL->Polltype == 0 ) {
				Packet = (u8*)malloc(128);
			} else {
				dbgprintf("HID: %u is an invalid Polltype value\n", CTRL->Polltype );
				return -4;
			}
		}

		if( CTRL->DPAD > 1 )
		{
			dbgprintf("HID: %u is an invalid DPAD value\n", CTRL->DPAD );
			return -5;
		}
	
		CTRL->Power.Offset	= ConfigGetValue( Data, "Power", 0 );
		CTRL->Power.Mask	= ConfigGetValue( Data, "Power", 1 );
	
		CTRL->A.Offset	= ConfigGetValue( Data, "A", 0 );
		CTRL->A.Mask	= ConfigGetValue( Data, "A", 1 );
	
		CTRL->B.Offset	= ConfigGetValue( Data, "B", 0 );
		CTRL->B.Mask	= ConfigGetValue( Data, "B", 1 );
	
		CTRL->X.Offset	= ConfigGetValue( Data, "X", 0 );
		CTRL->X.Mask	= ConfigGetValue( Data, "X", 1 );
	
		CTRL->Y.Offset	= ConfigGetValue( Data, "Y", 0 );
		CTRL->Y.Mask	= ConfigGetValue( Data, "Y", 1 );
	
		CTRL->Z.Offset	= ConfigGetValue( Data, "Z", 0 );
		CTRL->Z.Mask	= ConfigGetValue( Data, "Z", 1 );
	
		CTRL->L.Offset	= ConfigGetValue( Data, "L", 0 );
		CTRL->L.Mask	= ConfigGetValue( Data, "L", 1 );
	
		CTRL->R.Offset	= ConfigGetValue( Data, "R", 0 );
		CTRL->R.Mask	= ConfigGetValue( Data, "R", 1 );
	
		CTRL->S.Offset	= ConfigGetValue( Data, "S", 0 );
		CTRL->S.Mask	= ConfigGetValue( Data, "S", 1 );
	
	
		CTRL->Left.Offset	= ConfigGetValue( Data, "Left", 0 );
		CTRL->Left.Mask		= ConfigGetValue( Data, "Left", 1 );
	
		CTRL->Down.Offset	= ConfigGetValue( Data, "Down", 0 );
		CTRL->Down.Mask		= ConfigGetValue( Data, "Down", 1 );
	
		CTRL->Right.Offset	= ConfigGetValue( Data, "Right", 0 );
		CTRL->Right.Mask		= ConfigGetValue( Data, "Right", 1 );
	
		CTRL->Up.Offset		= ConfigGetValue( Data, "Up", 0 );
		CTRL->Up.Mask		= ConfigGetValue( Data, "Up", 1 );

		if( CTRL->DPAD )
		{	
			CTRL->RightUp.Offset	= ConfigGetValue( Data, "RightUp", 0 );
			CTRL->RightUp.Mask		= ConfigGetValue( Data, "RightUp", 1 );

			CTRL->DownRight.Offset	= ConfigGetValue( Data, "DownRight", 0 );
			CTRL->DownRight.Mask	= ConfigGetValue( Data, "DownRight", 1 );

			CTRL->DownLeft.Offset	= ConfigGetValue( Data, "DownLeft", 0 );
			CTRL->DownLeft.Mask		= ConfigGetValue( Data, "DownLeft", 1 );

			CTRL->UpLeft.Offset		= ConfigGetValue( Data, "UpLeft", 0 );
			CTRL->UpLeft.Mask		= ConfigGetValue( Data, "UpLeft", 1 );
		}
	
		CTRL->StickX	= ConfigGetValue( Data, "StickX", 0 );
		CTRL->StickY	= ConfigGetValue( Data, "StickY", 0 );
	
		CTRL->CStickX	= ConfigGetValue( Data, "CStickX", 0 );
		CTRL->CStickY	= ConfigGetValue( Data, "CStickY", 0 );
	
		CTRL->LAnalog	= ConfigGetValue( Data, "LAnalog", 0 );
		CTRL->RAnalog	= ConfigGetValue( Data, "RAnalog", 0 );
		
		dbgprintf("HID:Config file for VID:%04X PID:%04X loaded\n", CTRL->VID, CTRL->PID );

	} else {
		dbgprintf("HID:Failed to open config file:%u\n", ret );
		free(HIDHeap);
		return -2;
	}
	
	free(HIDHeap);

	return HIDHandle;
}
void HIDPS3Init(  struct SickSaxis *sicksaxis )
{
	memset32( req, 0, sizeof( req_args ) );

	char *buf = (char*)malloca( 0x20, 32 );
	memset32( buf, 0, 0x20 );

	req->device_no				= DeviceID;
	req->control.bmRequestType	= USB_REQTYPE_INTERFACE_GET;
	req->control.bmRequest		= USB_REQ_GETREPORT;
	req->control.wValue			= (USB_REPTYPE_FEATURE<<8) | 0xf2;
	req->control.wIndex			= 0;
	req->control.wLength		= 17;
	req->data					= buf;
		
	s32 ret = IOS_Ioctl( HIDHandle, /*ControlMessage*/2, req, 32, 0, 0 );
	if( ret < 0 )
	{
		dbgprintf("HID:HIDPS3Init:IOS_Ioctl( %u, %u, %p, %u, %u, %u):%d\n", HIDHandle, 2, req, 32, 0, 0, ret );
		Shutdown();
	}

	free(buf);
}
unsigned char rawData[49] =
{
    0x01, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xFF, 0x27, 0x10, 0x00, 0x32, 
    0xFF, 0x27, 0x10, 0x00, 0x32, 0xFF, 0x27, 0x10, 0x00, 0x32, 0xFF, 0x27, 0x10, 0x00, 0x32, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 
} ;

void HIDPS3SetLED( u8 led )
{	
	memset32( req, 0, sizeof( req_args ) );
	
	char *buf = (char*)malloca( 64, 32 );
	memset32( buf, 0, 64 );

	memcpy( buf, rawData, 49 );

    buf[10] = ss_led_pattern[led];
	
	req->device_no				= DeviceID;
	req->interrupt.dLength		= 49;
	req->interrupt.endpoint		= 0x02;
	req->data					= buf;

	s32 ret = IOS_Ioctl( HIDHandle, /*InterruptMessageIN*/4, req, 32, 0, 0 );
	if( ret < 0 ) dbgprintf("ES:IOS_Ioctl():%d\n", ret );
	
	free(buf);
}
void HIDPS3SetRumble( u8 duration_right, u8 power_right, u8 duration_left, u8 power_left)
{	
	memset32( req, 0, sizeof( req_args ) );
	
	char *buf = (char*)malloca( 64, 32 );
	memset32( buf, 0, 64 );

	memcpy( buf, rawData, 49 );
	
	buf[3] = power_left;
	buf[5] = power_right;
	
	req->device_no				= DeviceID;
	req->interrupt.dLength		= 49;
	req->interrupt.endpoint		= 0x02;
	req->data					= buf;

	s32 ret = IOS_Ioctl( HIDHandle, /*InterruptMessageIN*/4, req, 32, 0, 0 );
	if( ret < 0 )
		dbgprintf("ES:IOS_Ioctl():%d\n", ret );
	
	free(buf);
}
void HIDGCRead( void )
{
	PADStatus *Pad = (PADStatus*)0x10003FD0;
	memset32( Pad, 0, sizeof(PADStatus) * 4 );

	Pad[1].err = -1;	// NO controller
	Pad[2].err = -1;
	Pad[3].err = -1;
	
	u32 PADButtonsStick = *(vu32*)(0x0D806404);
	u32 PADTriggerCStick= *(vu32*)(0x0D806408);

	Pad[0].button = PADButtonsStick >> 16;

	if( (Pad[0].button & 0x234) == 0x234 )
		Shutdown();

	if( ((PADButtonsStick>>8)&0xFF) >= 255 )
	{
		Pad[0].stickX = 127;
	} else {
		Pad[0].stickX = ((PADButtonsStick>>8)&0xFF) - 127;
	}
	
	Pad[0].stickY = (PADButtonsStick&0xFF) - 127;
	
	Pad[0].substickX = ((PADTriggerCStick>>24)&0xFF)-128;
	Pad[0].substickY = ((PADTriggerCStick>>16)&0xFF)-128;
	
	Pad[0].triggerLeft = ((PADTriggerCStick>>8)&0xFF);
	Pad[0].triggerRight= ((PADTriggerCStick>>0)&0xFF);

	sync_after_write( Pad, 0x30 );
}
void HIDPS3Read( u8 *Data )
{	
	s32 ret;	
	memset32( req, 0, sizeof( req_args ) );
	memset32( Data, 0, SS_DATA_LEN );

	req->device_no				= DeviceID;
	req->control.bmRequestType	= USB_REQTYPE_INTERFACE_GET;
	req->control.bmRequest		= USB_REQ_GETREPORT;
	req->control.wValue			= (USB_REPTYPE_INPUT<<8) | 0x1;
	req->control.wIndex			= 0x0;
	req->control.wLength		= SS_DATA_LEN;
	req->data					= Data;

	ret = IOS_Ioctl( HIDHandle, /*ControlMessage*/2, req, 32, 0, 0 );
	if( ret < 0 )
	{
		dbgprintf("HID:HIDPS3Read:IOS_Ioctl( %u, %u, %p, %u, %u, %u):%d\n", HIDHandle, 2, req, 32, 0, 0, ret );
		Shutdown();
	}

	if( !PS3LedSet && Data[4] )
	{
		HIDPS3SetLED(1);
		PS3LedSet = 1;
	}
	
	return;
}
void HIDIRQRead( u8 *Data )
{
	s32 ret;
	
	memset32( req, 0, sizeof( req_args ) );
	memset32( Data, 0, wMaxPacketSize );

	req->device_no				= DeviceID;
	req->interrupt.dLength		= wMaxPacketSize;
	req->interrupt.endpoint		= bEndpointAddress;
	req->data					= Data;

	ret = IOS_Ioctl( HIDHandle, /*InterruptMessageIN*/3, req, 32, 0, 0 );
	if( ret < 0 )
	{
		dbgprintf("ES:HIDIRQRead:IOS_Ioctl():%d\n", ret );
		Shutdown();
	}

	return;
}
u32 HIDRumbeLast = 0;
void HIDRead( void )
{
	if( CTRL->Polltype )
	{
		HIDIRQRead( Packet );
	} else {
		HIDPS3Read( Packet );
	}

	if( CTRL->MultiIn )
	{
		if( Packet[0] != CTRL->MultiInValue )
			return;
	}
	
	if(Packet[CTRL->Power.Offset] & CTRL->Power.Mask )
		Shutdown();

	sync_before_read( (void*)0x10003FB0, 0x20 );

	if( HIDRumbeLast != *(vu32*)(0x10003FCC) )
	{
		HIDRumble( *(vu32*)(0x10003FCC) );
		HIDRumbeLast = *(vu32*)(0x10003FCC);
	}

	PADStatus *PAD = (PADStatus*)0x10003FD0;
	memset32( PAD, 0, sizeof(PADStatus) * 4 );
	
	PAD[1].err = -1;	// NO controller
	PAD[2].err = -1;
	PAD[3].err = -1;

	if( Packet[CTRL->A.Offset] & CTRL->A.Mask )
		PAD->button |= PAD_BUTTON_A;
			
	if( Packet[CTRL->B.Offset] & CTRL->B.Mask )
		PAD->button |= PAD_BUTTON_B;
			
	if( Packet[CTRL->X.Offset] & CTRL->X.Mask )
		PAD->button |= PAD_BUTTON_X;
			
	if( Packet[CTRL->Y.Offset] & CTRL->Y.Mask )
		PAD->button |= PAD_BUTTON_Y;
			
	if( Packet[CTRL->Z.Offset] & CTRL->Z.Mask )
		PAD->button |= PAD_TRIGGER_Z;
			
	if( Packet[CTRL->L.Offset] & CTRL->L.Mask )
		PAD->button |= PAD_TRIGGER_L;
			
	if( Packet[CTRL->R.Offset] & CTRL->R.Mask )
		PAD->button |= PAD_TRIGGER_R;
			
	if( Packet[CTRL->S.Offset] & CTRL->S.Mask )
		PAD->button |= PAD_BUTTON_START;
			
	PAD->stickX		= Packet[CTRL->StickX];
	PAD->stickY		= Packet[CTRL->StickY];
	PAD->substickX	= Packet[CTRL->CStickX];
	PAD->substickY	= Packet[CTRL->CStickY];
	PAD->triggerLeft= Packet[CTRL->LAnalog];
	PAD->triggerRight= Packet[CTRL->RAnalog];

	if( CTRL->DPAD == 0 )
	{
		if( Packet[CTRL->Left.Offset] & CTRL->Left.Mask )
			PAD->button |= PAD_BUTTON_LEFT;

		if( Packet[CTRL->Right.Offset] & CTRL->Right.Mask )
			PAD->button |= PAD_BUTTON_RIGHT;

		if( Packet[CTRL->Down.Offset] & CTRL->Down.Mask )
			PAD->button |= PAD_BUTTON_DOWN;

		if( Packet[CTRL->Up.Offset] & CTRL->Up.Mask )
			PAD->button |= PAD_BUTTON_UP;

	} else {

		if((Packet[CTRL->Up.Offset] == CTRL->Up.Mask)||(Packet[CTRL->UpLeft.Offset] == CTRL->UpLeft.Mask)||(Packet[CTRL->RightUp.Offset]	== CTRL->RightUp.Mask))
			PAD->button|=PAD_BUTTON_UP;				

		if(	(Packet[CTRL->Right.Offset] == CTRL->Right.Mask)||(Packet[CTRL->DownRight.Offset] == CTRL->DownRight.Mask)||(Packet[CTRL->RightUp.Offset] == CTRL->RightUp.Mask))
			PAD->button|=PAD_BUTTON_RIGHT;
				
		if((Packet[CTRL->Down.Offset] == CTRL->Down.Mask) ||(Packet[CTRL->DownRight.Offset] == CTRL->DownRight.Mask) ||(Packet[CTRL->DownLeft.Offset] == CTRL->DownLeft.Mask))				
			PAD->button|=PAD_BUTTON_DOWN;
				
		if((Packet[CTRL->Left.Offset] == CTRL->Left.Mask) || (Packet[CTRL->DownLeft.Offset] == CTRL->DownLeft.Mask) || (Packet[CTRL->UpLeft.Offset] == CTRL->UpLeft.Mask) )
			PAD->button |= PAD_BUTTON_LEFT;				
	}
	
	if( (u8)PAD->stickX >= 255 )
	{
		PAD->stickX = 127;
	} else {
		PAD->stickX = PAD->stickX - 127;
	}
	
	PAD->stickY = 127 - PAD->stickY;
	
	PAD->substickX = PAD->substickX - 128;
	PAD->substickY = 127 - PAD->substickY;

	sync_after_write( PAD, sizeof(PADStatus) * 4 );
}
void HIDRumble( u32 Enable )
{
	if( CTRL->Polltype == 0 )
	{
		switch( Enable )
		{
			case 0:	// stop
			case 2:	// hard stop
				HIDPS3SetRumble( 0, 0, 0, 0 );
			break;
			case 1: // start
				HIDPS3SetRumble( 0, 0xFF, 0, 1 );
			break;
		}
	}
}
void getdev( struct SickSaxis *sicksaxis )
{	
	s32 ret;
	memset32( req, 0, sizeof( req_args ) );
	
	char *buf = (char*)malloca( 32, 32 );
	memset32( buf, 0, 32 );

	req->device_no				= DeviceID;
	req->control.bmRequestType	= (1<<7);
	req->control.bmRequest		= USB_REQ_GETDESCRIPTOR;
	req->control.wValue			= (USB_REPTYPE_INPUT<<8);
	req->control.wIndex			= 0x0;
	req->control.wLength		= sizeof(usb_devdesc);
	req->data					= buf;

	ret = IOS_Ioctl( HIDHandle, /*ControlMessage*/2, req, 32, 0, 0 );
	dbgprintf("ES:IOS_Ioctl(%u):%d\n", sizeof(usb_devdesc), ret);
	
//	hexdump( buf, 30 );
	
	free(buf);
}
u32 ConfigGetValue( char *Data, const char *EntryName, u32 Entry )
{
	char entryname[128];
	_sprintf( entryname, "%s=", EntryName );

	char *str = strstr( Data, entryname );
	if( str == (char*)NULL )
	{
		dbgprintf("Entry:\"%s\" not found!\n", EntryName );
		return 0;
	}

	str += strlen(entryname); // Skip '='

	if( Entry == 0 )
	{
		return atox(str);

	} else if ( Entry == 1 ) {

		str = strstr( str, "," );
		if( str == (char*)NULL )
		{
			dbgprintf("No \",\" found in entry.\n");
			return 0;
		}

		str++; //Skip ,

		return atox(str);
	}

	return 0;
}