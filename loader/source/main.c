/*

Nintendont (Loader) - Playing Gamecubes in Wii mode on a Wii U

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

#include <stdio.h>
#include <stdlib.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <fat.h>
#include <sys/dir.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <stdarg.h>
#include <ctype.h>
#include <unistd.h>
#include <network.h>
#include <sys/errno.h>
#include <ogc/lwp_watchdog.h>
#include <ogc/isfs.h>
#include <ogc/ipc.h>

#include <dirent.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <fcntl.h>

#include "exi.h"
#include "dip.h"
#include "global.h"
#include "font.h"
#include "Config.h"
#include "FPAD.h"
#include "menu.h"
#include "loader.h"
#include "Patches.h"

#include "kernel_bin.h"

extern void ClearBats();

extern NIN_CFG ncfg;
extern FILE *cfg;
extern GXRModeObj *rmode;

void *Initialise();
void RAMInit( void );
void UpdateSRAM( int region );

GXRModeObj *vmode = NULL;

extern u32 Region;

s32 __IOS_LoadStartupIOS(void)
{
	int res;

	res = __ES_Init();
	if(res < 0)
		return res;

	return 0;
}

unsigned char Boot2Patch[20] =
{
    0x48, 0x03, 0x49, 0x04, 0x47, 0x78, 0x46, 0xC0, 0xE6, 0x00, 0x08, 0x70, 0xE1, 0x2F, 0xFF, 0x1E, 
    0x10, 0x10, 0x00, 0x00, 
};
unsigned char FSAccessPattern[8] =
{
    0x9B, 0x05, 0x40, 0x03, 0x99, 0x05, 0x42, 0x8B, 
} ;
unsigned char FSAccessPatch[8] =
{
    0x9B, 0x05, 0x40, 0x03, 0x1C, 0x0B, 0x42, 0x8B, 
};

int main(int argc, char **argv)
{	
	void	(*entrypoint)();
	
	CheckForGecko();

	if( !IsWiiU() )
	{
		gprintf("Nintendont Loader\n");
		gprintf("Built   : %s %s\n", __DATE__, __TIME__ );
		gprintf("Version : %d.%d\n", VERSION>>16, VERSION&0xFFFF );	
	}

	RAMInit();

	STM_RegisterEventHandler(HandleSTMEvent);

	Initialise();

	FPAD_Init();

	if( IsWiiU() )
	{
		PrintFormat( MENU_POS_X, MENU_POS_Y + 20*1, "Nintendont Loader (Wii U)         A: Start Game" );
	} else {
		PrintFormat( MENU_POS_X, MENU_POS_Y + 20*1, "Nintendont Loader (Wii)           A: Start Game" );
	}

	PrintFormat( MENU_POS_X, MENU_POS_Y + 20*2, "Built   : %s %s    B: Settings\n", __DATE__, __TIME__ );
	PrintFormat( MENU_POS_X, MENU_POS_Y + 20*3, "Firmware: %d.%d.%d\n", *(vu16*)0x80003140, *(vu8*)0x80003142, *(vu8*)0x80003143 );
	
	if( *(vu32*)(0xCd800064) != -1 )
	{
		ClearScreen();
		gprintf("Please load Nintendont with AHBProt disabled!\n");
		PrintFormat( 25, 232, "Please load Nintendont with AHBProt disabled!" );
		sleep(10);
		exit(1);
	}

	if( IsWiiU() )
	{
		if( *(vu16*)0x80003140 != 58 || *(vu8*)0x80003142 != 25 || *(vu8*)0x80003143 != 32 )
		{
			ClearScreen();
			gprintf("This version of IOS58 is not supported!\n");
			PrintFormat( 25, 232, "This version of IOS58 is not supported!" );
			sleep(10);
			exit(1);
		}

	} else {

		if( *(vu16*)0x80003140 != 58 || *(vu8*)0x80003142 != 24 || *(vu8*)0x80003143 != 32 )
		{
			ClearScreen();
			gprintf("This version of IOS58 is not supported!\n");
			PrintFormat( 25, 232, "This version of IOS58 is not supported!" );
			sleep(10);
			exit(1);
		}
	}
	
	*(vu32*)(0xCD8B420A) = 0;	// Disable MEM2 protection

//Patch FS access

	int u;
	for( u = 0x93A00000; u < 0x94000000; u+=2 )
	{
		if( memcmp( (void*)(u), FSAccessPattern, sizeof(FSAccessPattern) ) == 0 )
		{
		//	gprintf("FSAccessPatch:%08X\n", u );
			memcpy( (void*)u, FSAccessPatch, sizeof(FSAccessPatch) );
		}
	}

	fatInitDefault();	

	if( IsWiiU() )
	{
		gprintf("Built   : %s %s\n", __DATE__, __TIME__ );
		gprintf("Version : %d.%d\n", VERSION>>16, VERSION&0xFFFF );	
		gprintf("Firmware: %d.%d.%d\n", *(vu16*)0x80003140, *(vu8*)0x80003142, *(vu8*)0x80003143 );
	}
	
	u32 KernelSize = 0;
	u32 NKernelSize = 0;
	char *Kernel = (char*)0x80100000;

	if( LoadKernel( Kernel, &KernelSize ) < 0 )
	{
		ClearScreen();
		gprintf("Failed to load kernel from NAND!\n");
		PrintFormat( 25, 232, "Failed to load kernel from NAND!" );
		sleep(10);
		exit(1);
	}

	InsertModule( Kernel, KernelSize, (char*)kernel_bin, kernel_bin_size, (char*)0x90100000, &NKernelSize );

	DCFlushRange( (void*)0x90100000, NKernelSize );

//Load config

	u32 ConfigReset = 0;

	cfg = fopen("/nincfg.bin","rb+");
	if( cfg == NULL )
	{
		ConfigReset = 1;

	} else {

		if( fread( &ncfg, sizeof(NIN_CFG), 1, cfg ) != 1 )
			ConfigReset = 1;

		if( ncfg.Magicbytes != 0x01070CF6 )
			ConfigReset = 1;

		if( ncfg.Version != NIN_CFG_VERSION )
			ConfigReset = 1;		
	}

	if( ConfigReset )
	{
		cfg = fopen("/nincfg.bin","wb");

		memset( &ncfg, 0, sizeof(NIN_CFG) );
	
		ncfg.Magicbytes	= 0x01070CF6;
		ncfg.Version	= NIN_CFG_VERSION;
		ncfg.Language	= NIN_LAN_AUTO;
	}
	
//Reset drive

	if( ncfg.Config & NIN_CFG_AUTO_BOOT )
	{
		gprintf("Autobooting:\"%s\"\n", ncfg.GamePath );
		fclose( cfg );
	} else {
		SelectGame();
	}

	ClearScreen();
	

	if( IsWiiU() )
	{
		PrintFormat( MENU_POS_X, MENU_POS_Y + 20*1, "Nintendont Loader (Wii U)" );
	} else {
		PrintFormat( MENU_POS_X, MENU_POS_Y + 20*1, "Nintendont Loader (Wii)" );
	}

	PrintFormat( MENU_POS_X, MENU_POS_Y + 20*2, "Built   : %s %s\n", __DATE__, __TIME__ );
	PrintFormat( MENU_POS_X, MENU_POS_Y + 20*3, "Firmware: %d.%d.%d\n", *(vu16*)0x80003140, *(vu8*)0x80003142, *(vu8*)0x80003143 );
	
	WPAD_Disconnect(0);
	WPAD_Disconnect(1);
	WPAD_Disconnect(2);
	WPAD_Disconnect(3);

	WPAD_Shutdown();
	

	DCInvalidateRange( (void*)0x939F02F0, 0x20 );
			
	memcpy( (void*)0x939F02F0, Boot2Patch, sizeof(Boot2Patch) );

	DCFlushRange( (void*)0x939F02F0, 0x20 );
			
	s32 fd = IOS_Open( "/dev/es", 0 );
				
	u8 *buffer = (u8*)memalign( 32, 0x100 );
	memset( buffer, 0, 0x100 );
								
	memset( (void*)0x90004100, 0xFFFFFFFF, 0x20  );
	DCFlushRange( (void*)0x90004100, 0x20 );
								
	memset( (void*)0x91000000, 0xFFFFFFFF, 0x20  );
	DCFlushRange( (void*)0x91000000, 0x20 );

#ifdef DEBUG
	gprintf("ES_ImportBoot():");
#endif			

	u32 ret = IOS_IoctlvAsync( fd, 0x1F, 0, 0, (ioctlv*)buffer, NULL, NULL );
	
#ifdef DEBUG
	if( !IsWiiU() )
	{
		gprintf("%d\n", ret );
		gprintf("Waiting ...\n");
	}
#endif

	PrintFormat( MENU_POS_X, MENU_POS_Y + 20*10, "Loading patched kernel ...\n");
	while(1)
	{
		DCInvalidateRange( (void*)0x90004100, 0x20 );
		if( *(vu32*)(0x90004100) == 0xdeadbeef )
			break;

		PrintFormat( MENU_POS_X, MENU_POS_Y + 20*10, "Loading patched kernel ... %d\n", *(vu32*)(0x90004100) );
		
		VIDEO_WaitVSync();
	}
	
#ifdef DEBUG
	if( !IsWiiU() )
		gprintf("Nintendont at your service!\n");
#endif

	PrintFormat( MENU_POS_X, MENU_POS_Y + 20*11, "Nintendont kernel running, loading game ...\n");
//	memcpy( (void*)0x80000000, (void*)0x90140000, 0x1200000 );

	entrypoint = LoadGame();

#ifdef DEBUG
	gprintf("GameRegion:");
#endif

	if( ncfg.VideoMode & NIN_VID_FORCE )
	{
		gprintf("Force:%u (%02X)\n", ncfg.VideoMode & NIN_VID_FORCE, ncfg.VideoMode & NIN_VID_FORCE_MASK );

		switch( ncfg.VideoMode & NIN_VID_FORCE_MASK )
		{
			case NIN_VID_FORCE_NTSC:
			{
				*(vu32*)0x800000CC = 0;
				Region = 0;
			} break;
			case NIN_VID_FORCE_MPAL:
			{
				*(vu32*)0x800000CC = 3;
				Region = 2;
			} break;
			case NIN_VID_FORCE_PAL50:
			{
				*(vu32*)0x800000CC = 1;
				Region = 2;
			} break;
			case NIN_VID_FORCE_PAL60:
			{
				*(vu32*)0x800000CC = 5;
				Region = 2;
			} break;
		}
	}
	
	gprintf("Region:%u\n", Region );

	switch(Region)
	{
		default:
		case 0:
		case 1:
		{
#ifdef DEBUG
			gprintf("NTSC\n");
#endif
			*(vu32*)0x800000CC = 0;

			if( VIDEO_HaveComponentCable() )
				vmode = &TVNtsc480Prog;
			else
				vmode = &TVNtsc480IntDf;
			
		} break;
		case 2:
		{
			if( *(vu32*)0x800000CC == 5 )
			{
#ifdef DEBUG
				gprintf("PAL60\n");
#endif
				if( VIDEO_HaveComponentCable() )
					vmode = &TVEurgb60Hz480Prog;
				else
					vmode = &TVEurgb60Hz480IntDf;

			} else if( *(vu32*)0x800000CC == 3 ) {
#ifdef DEBUG
				gprintf("MPAL\n");
#endif
				if( VIDEO_HaveComponentCable() )
					vmode = &TVEurgb60Hz480Prog;
				else
					vmode = &TVMpal480IntDf;
			} else {
				
#ifdef DEBUG
				gprintf("PAL50\n");
#endif
				if( VIDEO_HaveComponentCable() )
					vmode = &TVEurgb60Hz480Prog;
				else
					vmode = &TVPal528IntDf;
			}

			*(vu32*)0x800000CC = 1;

		} break;
	}

	VIDEO_Configure( vmode );
	VIDEO_Flush();
	VIDEO_WaitVSync();

	ClearScreen();

	VIDEO_WaitVSync();
	
	*(u16*)(0xCC00501A) = 156;	// DSP refresh rate
	
	VIDEO_SetBlack(TRUE);

	VIDEO_Flush();

	settime(secs_to_ticks(time(NULL) - 927466348));

	IRQ_Disable();
				
	ICFlashInvalidate();
	
#ifdef DEBUG
	if( !IsWiiU() )
		gprintf("entrypoint(0x%08X)\n", entrypoint );
#endif

	entrypoint();
	while(1);

	return 0;
}

