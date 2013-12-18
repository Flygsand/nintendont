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
#include "global.h"
#include "EXI.h"
#include "ff.h"
#include "common.h"
#include "vsprintf.h"
#include "alloc.h"
#include "Patch.h"
#include "syscalls.h"
#include "Config.h"

extern u8 SRAM[64];
extern u32 Region;

u32 Device=0;
u32 SRAMWriteCount=0;
static u32 EXICommand = 0;
static u32 BlockOff= 0;
u8 *MCard = (u8 *)(0x11000000);
u32 CARDWriteCount = 0;
u32 IPLReadOffset;
FIL MemCard;

void EXIInit( void )
{
	u32 i,wrote,ret;
	
	write32( 0x0D80600C, 0 );
	write32( 0x0D806010, 0 );

//Create savefile dirs for the current game
	if( f_chdir("/saves") != FR_OK )
	{
		f_mkdir("/saves");
		f_chdir("/saves");
	}

	if( f_chdir((const TCHAR*)0) != FR_OK )
	{
		f_mkdir((const TCHAR*)0);
		f_chdir((const TCHAR*)0);
	}

	if( ConfigGetConfig(NIN_CFG_MEMCARDEMU) )
	{
		ret = f_open( &MemCard, "ninmem.raw", FA_OPEN_ALWAYS | FA_READ | FA_WRITE );
		if( ret != FR_OK )
		{
#ifdef DEBUG
			dbgprintf("EXI: Failed to open/create ninmem.raw:%u\n", ret );
#endif
			Shutdown();
		}

		if( MemCard.fsize == 0 )
		{
#ifdef DEBUG
			dbgprintf("EXI: Creating new memory card...");
#endif
			memset32( (void*)0x10800000, 0, 16*1024*1024 );

			f_write( &MemCard, (void*)0x10800000, 16*1024*1024, &wrote );

			f_sync( &MemCard );
		
#ifdef DEBUG
			dbgprintf("done\n");
#endif
		}
	
#ifdef DEBUG
		dbgprintf("EXI: Loading memory card...");
#endif

		f_lseek( &MemCard, 0 );
		f_read(  &MemCard, MCard, 16*1024*1024, &wrote );

#ifdef DEBUG
		dbgprintf("done\n");
#endif

		sync_after_write( MCard, 16*1024*1024 );
	}
	
	GC_SRAM *sram = (GC_SRAM*)SRAM;
	
	for( i=0; i < 12; ++i )
		sram->FlashID[0][i] = 0;

	sram->FlashIDChecksum[0] = 0xFF;

	sram->DisplayOffsetH = 0;
	sram->BootMode	&= ~0x40;	// Clear PAL60
	sram->Flags		&= ~3;		// Clear Videomode
	sram->Flags		&= ~0x80;	// Clear Progmode

	if( ConfigGetVideoMode() & NIN_VID_FORCE )
	{
		switch( ConfigGetVideoMode() & NIN_VID_FORCE_MASK )
		{
			case NIN_VID_FORCE_NTSC:
			{
				Region = 0;
			} break;
			case NIN_VID_FORCE_MPAL:
			case NIN_VID_FORCE_PAL50:
			case NIN_VID_FORCE_PAL60:
			{
				Region = 2;
			} break;
		}
	}

	switch(Region)
	{
		default:
		case 0:
		case 1:
		{
#ifdef DEBUG
			dbgprintf("SRAM:NTSC\n");
#endif
			*(vu32*)0xCC = 0;
			
		} break;
		case 2:
		{
#ifdef DEBUG
			if( *(vu32*)0xCC == 5 )
			{
				dbgprintf("SRAM:PAL60\n");
			} else {
				dbgprintf("SRAM:PAL50\n");
				*(vu32*)0xCC = 1;
			}
#endif
			sram->Flags		|= 1;
			sram->BootMode	|= 0x40;

		} break;
	}
	
	sram->Flags |= 0x80; // just always set this, unless the HW bit is set it won't switch to prog anyway
	
	SRAM_Checksum( (unsigned short *)SRAM, (unsigned short *)SRAM, (unsigned short *)( ((u8*)SRAM) + 2 ) );

}
void EXIShutdown( void )
{
	u32 wrote;
	
#ifdef DEBUG
		dbgprintf("EXI: Saving memory card...");
#endif

	sync_before_read( MCard, 16*1024*1024 );

	f_lseek( &MemCard, 0 );
	f_write( &MemCard, MCard, 16*1024*1024, &wrote );
	f_close( &MemCard );

#ifdef DEBUG
		dbgprintf("done\n");
#endif
}
u32 EXIDeviceMemoryCard( u8 *Data, u32 Length, u32 Mode )
{
	u32 EXIOK = 1;
	u32 read = 0;

	if( Mode == 1 )		// Write
	{
		switch( Length )
		{
			case 1:
			{
				if( EXICommand == MEM_BLOCK_READ || EXICommand == MEM_BLOCK_WRITE )
					break;

				switch( (u32)Data >> 24 )
				{
					case 0x00:
					{
						EXICommand = MEM_READ_ID_NINTENDO;
#ifdef DEBUG_EXI
						dbgprintf("EXI: CARDGetDeviceIDNintendo()\n");
#endif
					} break;
					case 0x89:
					{
#ifdef DEBUG_EXI
						dbgprintf("EXI: CARDClearStatus()\n");
#endif
					} break;
				}
			} break;
			case 2:
			{
				switch( (u32)Data >> 16 )
				{
					case 0x0000:
					{
						EXICommand = MEM_READ_ID;
#ifdef DEBUG_EXI
						dbgprintf("EXI: CARDGetDeviceID()\n");
#endif
					} break;
					case 0x8300:	//
					{
						EXICommand = MEM_READ_STATUS;
#ifdef DEBUG_EXI
						dbgprintf("EXI: CARDReadStatus()\n");
#endif
					} break;
					case 0x8101:
					{
#ifdef DEBUG_EXI
						dbgprintf("EXI: CARDIRQEnable()\n");
#endif
					} break;
					case 0x8100:
					{
#ifdef DEBUG_EXI
						dbgprintf("EXI: CARDIRQDisable()\n");
#endif
					} break;
				}
			} break;
			case 3:
			{
				switch( (u32)Data >> 24 )
				{
					case 0xF1:
					{
						BlockOff = (((u32)Data>>16)&0xFF)  << 17;
						BlockOff|= (((u32)Data>> 8)&0xFF)  << 9;
#ifdef DEBUG_EXI
						dbgprintf("EXI: CARDErasePage(%08X)\n", BlockOff );
#endif
						EXICommand = MEM_BLOCK_ERASE;
						CARDWriteCount = 0;
						write32( 0x10, 2 );			// EXI IRQ
						EXIOK = 2;
					} break;
				}
			} break;
			case 4:
			{
				if( EXICommand == MEM_BLOCK_READ || EXICommand == MEM_BLOCK_WRITE )
					break;

				switch( (u32)Data >> 24 )
				{
					case 0xF1:
					{
						BlockOff = (((u32)Data>>16)&0xFF)  << 17;
						BlockOff|= (((u32)Data>> 8)&0xFF)  << 9;
						BlockOff|= (((u32)Data&0xFF) & 3 ) << 7;
#ifdef DEBUG_EXI
						dbgprintf("EXI: CARDErasePage(%08X)\n", BlockOff );
#endif
						EXICommand = MEM_BLOCK_ERASE;
						CARDWriteCount = 0;
						write32( 0x10, 2 );			// EXI IRQ
						EXIOK = 2;
					} break;
					case 0xF2:
					{
						BlockOff = (((u32)Data>>16)&0xFF)  << 17;
						BlockOff|= (((u32)Data>> 8)&0xFF)  << 9;
						BlockOff|= (((u32)Data&0xFF) & 3 ) << 7;
#ifdef DEBUG_EXI
						dbgprintf("EXI: CARDWritePage(%08X)\n", BlockOff );
#endif

						EXICommand = MEM_BLOCK_WRITE;
					} break;
					case 0x52:
					{
						BlockOff = (((u32)Data>>16)&0xFF)  << 17;
						BlockOff|= (((u32)Data>> 8)&0xFF)  << 9;
						BlockOff|= (((u32)Data&0xFF) & 3 ) << 7;							
#ifdef DEBUG_EXI
						dbgprintf("EXI: CARDReadPage(%08X)\n", BlockOff );
#endif

						EXICommand = MEM_BLOCK_READ;
					} break;
#ifdef DEBUG
					default:
					{
						dbgprintf("EXI: Unknown:%08x Line:%u\n", (u32)Data, __LINE__ );
					//	Shutdown();
					} break;
#endif
				}			
			} break;
			default:
			{
				switch( EXICommand )
				{
					case MEM_BLOCK_WRITE:
					{
						sync_before_read( Data, Length );

						memcpy( MCard+BlockOff, Data, Length );

						sync_after_write( MCard+BlockOff, Length );

						write32( 0x10, 10 );	// TC(8) & EXI(2) IRQ

						EXIOK = 2;
					} break;
				}
			} break;
		}

	} else {			// Read

		switch( EXICommand )
		{
			case MEM_READ_ID_NINTENDO:
			case MEM_READ_ID:
			{
				if( ConfigGetConfig(NIN_CFG_MEMCARDEMU) )
				{
					write32( 0x0D806010, 0x00000080 );
				} else {
					write32( 0x0D806010, 0x00000000 );
				}
#ifdef DEBUG_EXI
				dbgprintf("EXI: CARDReadID(%X)\n", read32(0x0D806010) );
#endif
			} break;
			case MEM_READ_STATUS:
			{
				write32( 0x0D806010, 0x41 );	// Unlocked(0x40) and Ready(0x01)
#ifdef DEBUG_EXI
				dbgprintf("EXI: CARDReadStatus(%X)\n", read32(0x0D806010) );
#endif
			} break;
			case MEM_BLOCK_READ:
			{
			//	f_lseek( &MemCard, BlockOff );
			//	f_read( &MemCard, Data, Length, &read );

				memcpy( Data, MCard+BlockOff, Length );

				sync_after_write( Data, Length );

				write32( 0x10, 8 );		// TC IRQ

				EXIOK = 2;
			} break;
		}
	}
	
	write32( 0x0D80600C, 0 );

	if( EXIOK == 2 )
	{		
		write32( 0x14, 0x10 );		// EXI(TC) IRQ
	
		sync_after_write( 0, 0x20 );

		write32( 0x0d80000C, (1<<0) | (1<<4) );
		write32( HW_PPCIRQFLAG, read32(HW_PPCIRQFLAG) );
		write32( HW_ARMIRQFLAG, read32(HW_ARMIRQFLAG) );
		set32( 0x0d80000C, (1<<2) );
	}

	return 1;
}
u32 EXIDevice_ROM_RTC_SRAM_UART( u8 *Data, u32 Length, u32 Mode )
{
	u32 read;

	if( Mode == 1 )		// Write
	{
		switch( Length )
		{
			case 4:
			{
				if( EXICommand == SRAM_WRITE )
				{
					*(u32*)(SRAM+SRAMWriteCount) = (u32)Data;

					SRAMWriteCount += Length;

					break;
				}

				if( (u32)Data == 0x20000100 )
				{
					EXICommand = SRAM_READ;
#ifdef DEBUG_EXI
					dbgprintf("EXI: SRAMRead()\n");
#endif
					break;
				}

				if( (u32)Data == 0xA0000100 || (u32)Data == 0xA0000600 )
				{
					EXICommand = SRAM_WRITE;
					SRAMWriteCount = 0;
#ifdef DEBUG_EXI
					dbgprintf("EXI: SRAMWrite()\n");
#endif
					break;
				}

				if( ((u32)Data >> 6 ) >= 0x1FCF00 )
				{
					EXICommand = IPL_READ_FONT_ANSI;
#ifdef DEBUG_EXI
					dbgprintf("EXI: IPLReadFont()\n");
#endif
					IPLReadOffset = (u32)Data >> 6;
					break;
				}

			} break;
		}

	} else {

		switch( EXICommand )
		{
			case IPL_READ_FONT_ANSI:
			{
				FIL ipl;
				if( f_open( &ipl, "/ipl.bin", FA_OPEN_EXISTING|FA_READ ) == FR_OK )
				{
					f_lseek( &ipl, IPLReadOffset );
					f_read( &ipl, Data, Length, &read );
					f_close( &ipl );

					sync_after_write( Data, Length );	
#ifdef DEBUG_EXI
					dbgprintf("EXI: IPLRead( %p, %08X, %u)\n", Data, IPLReadOffset, Length );
#endif
				}
			} break;
			case SRAM_READ:
			{
				memcpy( Data, SRAM, Length );

				sync_after_write( Data, Length );
						
#ifdef DEBUG_EXI
				dbgprintf("EXI: SRAMRead(%p,%u)\n", Data, Length );
#endif
			} break;
#ifdef DEBUG
			default:
			{
				dbgprintf("EXI: Unknown:%08x Line:%u\n", (u32)Data, __LINE__ );
				Shutdown();
			} break;
#endif
		}
	}

	write32( 0x0D80600C, 0 );

	return 1;
}
u32 EXIDeviceSP1( u8 *Data, u32 Length, u32 Mode )
{
	if( Mode == 1 )		// Write
	{
		switch( Length )
		{
			case 2:
			{
				switch( (u32)Data >>16 )
				{
					case 0x0000:
					{
						EXICommand = MEM_READ_ID;
					} break;
				}
			} break;
		}

	} else {
		switch(EXICommand)
		{
			case MEM_READ_ID:
			{
				write32( 0x0D806010, 0 );
			} break;
		}
	}

	write32( 0x0D80600C, 0 );

	return 0;
}
void EXIUpdateRegistersNEW( void )
{
	u32 i;

	u32 command = read32(0x0D80600C);
	
	if( command & 0xFF000000 )
	{
		switch( command >> 24 )
		{
			case 0x10:	// EXISelect
			{
				u32 chn	=  command & 0xFF;
				u32 dev = (command>>8) & 0xFF;
				u32 frq = (command>>16) & 0xFF;
				
#ifdef DEBUG_EXI
			//	dbgprintf("EXISelect( %u, %u, %u )\n", chn, dev, frq );
#endif
				u32 ret = 1;

				switch( chn )
				{
					case 0:
					{
						switch( dev )
						{
							case 0:
							{
								Device = EXI_DEV_MEMCARD_A;
							} break;
							case 1:
							{
								Device = EXI_DEV_MASK_ROM_RTC_SRAM_UART;
							} break;
							case 2:
							{
								Device = EXI_DEV_SP1;
							} break;
						}
					} break;
					case 1:
					{
						Device = EXI_DEV_MEMCARD_B;
						ret = 0;
					} break;
					case 2:
					{
						Device = EXI_DEV_AD16;
						ret = 0;
					} break;
				}

				EXICommand = 0;
				
				write32( 0x0D806010, ret );
				write32( 0x0D80600C, 0 );

			} break;
			case 0x11:	// EXI_Imm( s32 nChn, void *pData, u32 nLen, u32 nMode, EXICallback tc_cb );
			{
				u32 chn	=	(command >> 20) & 0xF;
				u32 data=	read32(0x0D806010);
				u32 len	=	command& 0xFFFF;
				u32 mode=	(command >> 16) & 0xF;

				if( len > 4 )
				{
					data = P2C(data);
				}
				
#ifdef DEBUG_EXI
			//	dbgprintf("EXIImm( %u, %p, %u, %u, Dev:%u EC:%u )\n", chn, data, len, mode, Device, EXICommand );
#endif
				switch( Device )
				{
					case EXI_DEV_MEMCARD_A:
					{
						EXIDeviceMemoryCard( (u8*)data, len, mode );
					} break;
					case EXI_DEV_MASK_ROM_RTC_SRAM_UART:
					{
						EXIDevice_ROM_RTC_SRAM_UART( (u8*)data, len, mode );
					} break;
					case EXI_DEV_SP1:
					{
						EXIDeviceSP1( (u8*)data, len, mode );
					} break;
					default:
					{
#ifdef DEBUG
						dbgprintf("EXI: EXIImm: Unhandled device:%u\n", Device );
#endif
					} break;
				}

			} break;
			case 0x12:	// EXIDMA
			{
				u32 chn	=	(command >> 20) & 0xF;
				u8 *ptr=	(u8*)P2C(read32(0x0D806010));
				u32 len	=	command& 0xFFFF;
				u32 mode=	(command >> 16) & 0xF;
				
#ifdef DEBUG_EXI
		//		dbgprintf("EXIDMA( %u, %p, %u, %u )\n", chn, ptr, len, mode );
#endif
				switch( Device )
				{
					case EXI_DEV_MEMCARD_A:
					{
						EXIDeviceMemoryCard( ptr, len, mode );
					} break;
					case EXI_DEV_MASK_ROM_RTC_SRAM_UART:
					{
						EXIDevice_ROM_RTC_SRAM_UART( ptr, len, mode );
					} break;
					case EXI_DEV_SP1:
					{
#ifdef DEBUG
						hexdump( ptr, len );
#endif
						EXIDeviceSP1( ptr, len, mode );
					} break;
					default:
					{
#ifdef DEBUG
						dbgprintf("EXI: EXIDMA: Unhandled device:%u\n", Device );
#endif
					} break;
				}

				EXICommand = 0;

			} break;
			default:
			{
			} break;
		}		
	}
}

