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
#include "string.h"
#include "syscalls.h"
#include "global.h"
#include "ipc.h"
#include "common.h"
#include "alloc.h"
#include "DI.h"
#include "ES.h"
#include "StreamADPCM.h"
#include "HID.h"
#include "EXI.h"
#include "SDI.h"

int verbose = 0;
u32 base_offset=0;
void *queuespace=NULL;
int queueid = 0;
int heapid=0;
int FFSHandle=0;
u32 FSUSB=0;
FIL GameFile;

//#undef DEBUG

extern u16 TitleVersion;
extern u32 *KeyID;
extern u8 *CNTMap;
extern u32 *HCR;
extern u32 *SDStatus;

extern u32 Streaming;
extern u32 StreamOffset;
extern s32 StreamSize;
extern u32 StreamTimer;
extern vu32 SDisInit;
extern u32 DiscChangeIRQ;

extern int dbgprintf( const char *fmt, ...);

FATFS *fatfs;

u32 Loopmode=0;
int _main( int argc, char *argv[] )
{
	s32 ret=0;
	u8 MessageHeap[0x10];
	u32 MessageQueue=0xFFFFFFFF;

	BootStatus(0);

	thread_set_priority( 0, 0x79 );	// do not remove this, this waits for FS to be ready!
	thread_set_priority( 0, 0x50 );
	thread_set_priority( 0, 0x79 );
	
	dbgprintf("Nintendont\n");

	dbgprintf("Built   : %s %s\n", __DATE__, __TIME__ );
	dbgprintf("Version : %d.%d\n", VERSION>>16, VERSION&0xFFFF );	

	MessageQueue = ES_Init( MessageHeap );

	BootStatus(1);

	ret = SDHCInit();
	if(!ret)
	{
#ifdef DEBUG
		dbgprintf("SD:SDHCInit() failed:%d\n", ret );
#endif
		BootStatus(-1);
		Shutdown();
	}

	BootStatus(2);

	fatfs = (FATFS*)malloca( sizeof(FATFS), 32 );

	s32 res = f_mount( 0, fatfs );
	if( res != FR_OK )
	{
#ifdef DEBUG
		dbgprintf("ES:f_mount() failed:%d\n", res );
#endif	
		BootStatus(-2);
		Shutdown();
	}
	
	BootStatus(3);

	ConfigInit();
	
	BootStatus(4);

	SDisInit = 1;
	
	ret = 0;
	u32 PADTimer=0;
			
	thread_set_priority( 0, 0x0A );

	if( ConfigGetConfig(NIN_CFG_HID) )
	{
		if( HIDInit() < 0 )
		{
#ifdef DEBUG
			dbgprintf("ES:HIDInit() failed\n" );
#endif
			BootStatus(-3);
			Shutdown();
		}
	}

	BootStatus(5);
	
	PADTimer = read32( HW_TIMER );		

	DIinit();
	BootStatus(6);

	EXIInit();

	BootStatus(7);

//Tell PPC side we are ready!	
	BootStatus(0xdeadbeef);
	
	write32( HW_PPCIRQFLAG, read32(HW_PPCIRQFLAG) );
	write32( HW_ARMIRQFLAG, read32(HW_ARMIRQFLAG) );
	
	set32( HW_PPCIRQMASK, (1<<31) );
	set32( HW_IPC_PPCCTRL, 0x30 );
												
	thread_set_priority( 0, 127 );

	cc_ahbMemFlush(1);

	write32(0xd8006a0, 0x30000004), mask32(0xd8006a8, 0, 2);
		
	while (1)
	{
		_ahbMemFlush(0);

		if( (u64)( read32(HW_TIMER) - PADTimer ) >= 50000 )	// about 37 times a second, 63287
		{
			if( ConfigGetConfig(NIN_CFG_HID) )
			{
				HIDRead();
			} else {
				HIDGCRead();
			}

			PADTimer = read32(HW_TIMER);				
		}

		//Baten Kaitos save hax
		if( read32(0) == 0x474B4245 )
		{
			if( read32( 0x0073E640 ) == 0xFFFFFFFF )
			{
				write32( 0x0073E640, 0 );
			}
		}

		if( Streaming )
		{
			if( (*(vu32*)0x0d800010 * 19 / 10) - StreamTimer >= 5000000 )
			{
			//	dbgprintf(".");
				StreamOffset += 64*1024;

				if( StreamOffset >= StreamSize )
				{
					StreamOffset = StreamSize;
					Streaming = 0;
				}
				StreamTimer = read32(HW_TIMER) * 19 / 10;
			}
		}

		if( DiscChangeIRQ )
		if( read32(HW_TIMER) * 128 / 243000000 > 2 )
		{
		//	dbgprintf("DIP:IRQ mon!\n");

			while( read32(DI_SCONTROL) & 1 )
				clear32( DI_SCONTROL, 1 );

			set32( DI_SSTATUS, 0x3A );

			write32( 0x0d80000C, (1<<0) | (1<<4) );
			write32( HW_PPCIRQFLAG, read32(HW_PPCIRQFLAG) );
			write32( HW_ARMIRQFLAG, read32(HW_ARMIRQFLAG) );
			set32( 0x0d80000C, (1<<2) );	

			DiscChangeIRQ = 0;
		}

		DIUpdateRegisters();

		EXIUpdateRegistersNEW();

		cc_ahbMemFlush(1);
	}

	return 0;
}
