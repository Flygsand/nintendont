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
#include "menu.h"
#include "font.h"
#include "exi.h"
#include "global.h"
#include "FPad.h"
#include "Config.h"
#include <dirent.h>
#include <sys/dir.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ogc/stm.h>
#include <ogc/video.h>
#include <ogc/video_types.h>
#include <ogc/consol.h>
#include <ogc/system.h>
#include <fat.h>

extern NIN_CFG ncfg;
extern FILE *cfg;

u32 Shutdown = 0;
void HandleWiiMoteEvent(s32 chan)
{
	Shutdown = 1;
}
void HandleSTMEvent(u32 event)
{
	*(vu32*)(0xCC003024) = 1;

	switch(event)
	{
		default:
		case STM_EVENT_RESET:
			break;
		case STM_EVENT_POWER:
			Shutdown = 1;
			break;
	}
}
void SelectGame( void )
{
//Create a list of games
	char filename[MAXPATHLEN];
	
	DIR *pdir;
	struct dirent *pent;
	struct stat statbuf;

	pdir = opendir ("/games");
	if( !pdir )
	{
		ClearScreen();
		gprintf("No FAT device found, or missing games dir!\n");
		PrintFormat( 25, 232, "No FAT device found, or missing games dir!" );
		sleep(10);
		exit(1);
	}

	u32 gamecount = 0;
	char buf[0x100];
	gameinfo gi[64];

	memset( gi, 0, sizeof(gameinfo) * 64 );

	while( ( pent = readdir(pdir) ) != NULL )
	{
		stat( pent->d_name, &statbuf );
		if( pent->d_type == DT_DIR )
		{
			if( strstr( pent->d_name, "." ) != NULL )
				continue;

		//	gprintf( "%s", pent->d_name );

			//Test if game.iso exists and add to list

			sprintf( filename, "sd:/games/%s/game.iso", pent->d_name );

			FILE *in = fopen( filename, "rb" );
			if( in != NULL )
			{
			//	gprintf("(%s) ok\n", filename );
				fread( buf, 1, 0x100, in );
				fclose(in);

				if( *(vu32*)(buf+0x1C) == 0xC2339F3D )	// Must be GC game
				{
					gi[gamecount].Name = strdup( buf + 0x20 );
					gi[gamecount].Path = strdup( filename );

					gamecount++;
				}
			} else { // Check for FST format
				
				sprintf( filename, "sd:/games/%s/sys/boot.bin", pent->d_name );

				FILE *in = fopen( filename, "rb" );
				if( in != NULL )
				{
				//	gprintf("(%s) ok\n", filename );
					fread( buf, 1, 0x100, in );
					fclose(in);

					if( *(vu32*)(buf+0x1C) == 0xC2339F3D )	// Must be GC game
					{
						sprintf( filename, "sd:/games/%s/", pent->d_name );

						gi[gamecount].Name = strdup( buf + 0x20 );
						gi[gamecount].Path = strdup( filename );

						gamecount++;
					}
				}
			}
		}
	}

	if( gamecount == 0 )
	{
		ClearScreen();
		gprintf("No games found!\n");
		PrintFormat( 25, 232, "No games found!" );
		sleep(10);
		exit(1);
	}

	u32 redraw = 1;
	u32 i;
	s32 PosX = 0;
	s32 ScrollX = 0;
	u32 MenuMode = 0;

	u32 ListMax = gamecount;
	if( ListMax > 14 )
		ListMax = 14;

	while(1)
	{
		FPAD_Update();

		if( FPAD_Start(1) )
		{
			ClearScreen();
			PrintFormat( 90, 232, "Returning to loader..." );
			exit(0);			
		}

		if( FPAD_Cancel(0) )
		{
			MenuMode ^= 1;

			PosX	= 0;
			ScrollX = 0;

			if( MenuMode == 0 )
			{
				ListMax = gamecount;
				if( ListMax > 14 )
					ListMax = 14;

			}  else {

				ListMax = 9;

				if( (ncfg.VideoMode & NIN_VID_MASK) == NIN_VID_FORCE )
					ListMax = 10;
			}
			
			redraw = 1;

			ClearScreen();

			if( IsWiiU() )
			{
				PrintFormat( MENU_POS_X, MENU_POS_Y + 20*1, "Nintendont Loader (Wii U)         A: Start Game" );
			} else {
				PrintFormat( MENU_POS_X, MENU_POS_Y + 20*1, "Nintendont Loader (Wii)           A: Start Game" );
			}

			PrintFormat( MENU_POS_X, MENU_POS_Y + 20*2, "Built   : %s %s    B: Settings\n", __DATE__, __TIME__ );
			PrintFormat( MENU_POS_X, MENU_POS_Y + 20*3, "Firmware: %d.%d.%d\n", *(vu16*)0x80003140, *(vu8*)0x80003142, *(vu8*)0x80003143 );
		}

	//	gprintf("\rS:%u P:%u G:%u M:%u    ", ScrollX, PosX, gamecount, ListMax );

		if( MenuMode == 0 )
		{
			if( FPAD_Down(0) )
			{
				PrintFormat( MENU_POS_X+51*6-8, MENU_POS_Y + 20*6 + PosX * 20, " " );

				if( PosX + 1 >= ListMax )
				{
					if( PosX + 1 + ScrollX < gamecount)
						ScrollX++;
					else {
						PosX	= 0;
						ScrollX = 0;
					}
				} else {
					PosX++;
				}
			
				redraw=1;
			} else if( FPAD_Up(0) )
			{
				PrintFormat( MENU_POS_X+51*6-8, MENU_POS_Y + 20*6 + PosX * 20, " " );

				if( PosX <= 0 )
				{
					if( ScrollX > 0 )
						ScrollX--;
					else {
						PosX	= ListMax - 1;
						ScrollX = gamecount - ListMax;
					}
				} else {
					PosX--;
				}

				redraw=1;
			}

			if( FPAD_OK(0) )
			{
				break;
			}

			if( redraw )
			{
				for( i=0; i < ListMax; ++i )
					PrintFormat( MENU_POS_X-8, MENU_POS_Y + 20*6 + i * 20, "% 51.51s", gi[i+ScrollX].Name );
					
				PrintFormat( MENU_POS_X+51*6-8, MENU_POS_Y + 20*6 + PosX * 20, "<" );
			}

		} else {		
			
			if( FPAD_Down(0) )
			{
				PrintFormat( MENU_POS_X+30, 164+16*PosX, " " );
				
				PosX++;

				if( PosX >= ListMax )
				{
					ScrollX = 0;
					PosX	= 0;					
				}
			
				redraw=1;

			} else if( FPAD_Up(0) )
			{
				PrintFormat( MENU_POS_X+30, 164+16*PosX, " " );

				PosX--;

				if( PosX < 0 )
					PosX = ListMax - 1;			

				redraw=1;
			}

			if( FPAD_OK(0) )
			{
				switch( PosX )
				{
					case 0:
					{
						ncfg.Config ^= NIN_CFG_CHEATS;
					} break;
					case 1:
					{
						ncfg.Config ^= NIN_CFG_FORCE_PROG;
					} break;
					case 2:
					{
						ncfg.Config ^= NIN_CFG_FORCE_WIDE;
					} break;
					case 3:
					{
						ncfg.Config ^= NIN_CFG_MEMCARDEMU;
					} break;
					case 4:
					{
						ncfg.Config ^= NIN_CFG_DEBUGGER;
					} break;
					case 5:
					{
						ncfg.Config ^= NIN_CFG_DEBUGWAIT;
					} break;
					case 6:
					{
						ncfg.Config ^= NIN_CFG_HID;
					} break;
					case 7:
					{
						ncfg.Config ^= NIN_CFG_OSREPORT;
					} break;
					case 8:
					{
						switch( ncfg.VideoMode & NIN_VID_MASK )
						{
							case NIN_VID_AUTO:
								ncfg.VideoMode &= ~NIN_VID_MASK;
								ncfg.VideoMode |= NIN_VID_FORCE;

								ListMax = 10;
							break;
							case NIN_VID_FORCE:
								ncfg.VideoMode &= ~NIN_VID_MASK;
								ncfg.VideoMode |= NIN_VID_NONE;

								ListMax = 9;

								PrintFormat( MENU_POS_X+50, 164+16*9, "                             " );

							break;
							case NIN_VID_NONE:
								ncfg.VideoMode &= ~NIN_VID_MASK;
								ncfg.VideoMode |= NIN_VID_AUTO;

								ListMax = 9;

								PrintFormat( MENU_POS_X+50, 164+16*9, "                             " );

							break;
						}

					} break;
					case 9:
					{
						switch( ncfg.VideoMode & NIN_VID_FORCE_MASK )
						{
							case NIN_VID_FORCE_PAL50:
								ncfg.VideoMode &= ~NIN_VID_FORCE_MASK;
								ncfg.VideoMode |= NIN_VID_FORCE_PAL60;
							break;
							case NIN_VID_FORCE_PAL60:
								ncfg.VideoMode &= ~NIN_VID_FORCE_MASK;
								ncfg.VideoMode |= NIN_VID_FORCE_NTSC;
							break;
							case NIN_VID_FORCE_NTSC:
								ncfg.VideoMode &= ~NIN_VID_FORCE_MASK;
								ncfg.VideoMode |= NIN_VID_FORCE_MPAL;
							break;
							case NIN_VID_FORCE_MPAL:
								ncfg.VideoMode &= ~NIN_VID_FORCE_MASK;
								ncfg.VideoMode |= NIN_VID_FORCE_PAL50;
							break;
						}

					} break;
				}
			}

			if( redraw )
			{
				PrintFormat( MENU_POS_X+50, 164+16*0, "Cheats                 :%s", (ncfg.Config&NIN_CFG_CHEATS)		? "On " : "Off" );
				PrintFormat( MENU_POS_X+50, 164+16*1, "Force Progressive      :%s", (ncfg.Config&NIN_CFG_FORCE_PROG)	? "On " : "Off" );
				PrintFormat( MENU_POS_X+50, 164+16*2, "Force Widescreen       :%s", (ncfg.Config&NIN_CFG_FORCE_WIDE)	? "On " : "Off" );
				PrintFormat( MENU_POS_X+50, 164+16*3, "Memcard Emulation      :%s", (ncfg.Config&NIN_CFG_MEMCARDEMU)		? "On " : "Off" );
				PrintFormat( MENU_POS_X+50, 164+16*4, "Debugger          (NYI):%s", (ncfg.Config&NIN_CFG_DEBUGGER)		? "On " : "Off" );
				PrintFormat( MENU_POS_X+50, 164+16*5, "Debugger Wait     (NYI):%s", (ncfg.Config&NIN_CFG_DEBUGWAIT)		? "On " : "Off" );
				PrintFormat( MENU_POS_X+50, 164+16*6, "Use HID device         :%s", (ncfg.Config&NIN_CFG_HID)			? "On " : "Off" );
				PrintFormat( MENU_POS_X+50, 164+16*7, "OSReport               :%s", (ncfg.Config&NIN_CFG_OSREPORT)		? "On " : "Off" );

				switch( ncfg.VideoMode & NIN_VID_MASK )
				{
					case NIN_VID_AUTO:
						PrintFormat( MENU_POS_X+50, 164+16*8,"Video                  :%s", "auto " );
					break;
					case NIN_VID_FORCE:
						PrintFormat( MENU_POS_X+50, 164+16*8,"Video                  :%s", "force" );
					break;
					case NIN_VID_NONE:
						PrintFormat( MENU_POS_X+50, 164+16*8,"Video                  :%s", "none " );
					break;		
					default:
						ncfg.VideoMode &= ~NIN_VID_MASK;
						ncfg.VideoMode |= NIN_VID_AUTO;
					break;			
				}

				if( (ncfg.VideoMode & NIN_VID_FORCE) == NIN_VID_FORCE )
				switch( ncfg.VideoMode & NIN_VID_FORCE_MASK )
				{
					case NIN_VID_FORCE_PAL50:
						PrintFormat( MENU_POS_X+50, 164+16*9, "Videomode              :%s", "PAL50" );
					break;
					case NIN_VID_FORCE_PAL60:
						PrintFormat( MENU_POS_X+50, 164+16*9, "Videomode              :%s", "PAL60" );
					break;
					case NIN_VID_FORCE_NTSC:
						PrintFormat( MENU_POS_X+50, 164+16*9, "Videomode              :%s", "NTSC " );
					break;
					case NIN_VID_FORCE_MPAL:
						PrintFormat( MENU_POS_X+50, 164+16*9, "Videomode              :%s", "MPAL " );
					break;
					default:
						ncfg.VideoMode &= ~NIN_VID_FORCE_MASK;
						ncfg.VideoMode |= NIN_VID_FORCE_NTSC;
					break;
				}

				PrintFormat( MENU_POS_X+30, 164+16*PosX, ">" );
			}
		}
		
		VIDEO_WaitVSync();
	}

	memcpy( ncfg.GamePath, gi[PosX+ScrollX].Path+3, strlen(gi[PosX+ScrollX].Path) );

	for( i=0; i < gamecount; ++i )
	{
		free(gi[i].Name);
		free(gi[i].Path);
	}

	fseek( cfg, 0, 0);
	fwrite( &ncfg, sizeof(NIN_CFG), 1, cfg );
	fclose( cfg );
}