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
#ifndef _CFG_
#define _CFG_

#include "global.h"

#define NIN_CFG_VERSION		0x00000001

typedef struct NIN_CFG 
{
	u32		Magicbytes;		// 0x01070CF6
	u32		Version;		// 0x00000001
	u32		Config;
	u32		VideoMode;
	u32		Language;
	char	GamePath[255];
	char	CheatPath[255];
} NIN_CFG;

enum ninconfig
{
	NIN_CFG_CHEATS		= (1<<0),
	NIN_CFG_DEBUGGER	= (1<<1),	// Only for Wii Version
	NIN_CFG_DEBUGWAIT	= (1<<2),	// Only for Wii Version
	NIN_CFG_MEMCARDEMU	= (1<<3),
	NIN_CFG_CHEAT_PATH	= (1<<4),
	NIN_CFG_FORCE_WIDE	= (1<<5),
	NIN_CFG_FORCE_PROG	= (1<<6),
	NIN_CFG_AUTO_BOOT	= (1<<7),
	NIN_CFG_HID			= (1<<8),
	NIN_CFG_OSREPORT	= (1<<9),
};

enum ninvideomode
{
	NIN_VID_AUTO		= (0<<16),
	NIN_VID_FORCE		= (1<<16),
	NIN_VID_NONE		= (2<<16),

	NIN_VID_MASK		= NIN_VID_AUTO|NIN_VID_FORCE|NIN_VID_NONE,

	NIN_VID_FORCE_PAL50	= (1<<0),
	NIN_VID_FORCE_PAL60	= (1<<1),
	NIN_VID_FORCE_NTSC	= (1<<2),
	NIN_VID_FORCE_MPAL	= (1<<3),

	NIN_VID_FORCE_MASK	= NIN_VID_FORCE_PAL50|NIN_VID_FORCE_PAL60|NIN_VID_FORCE_NTSC|NIN_VID_FORCE_MPAL,
};

enum ninlanguage
{
	NIN_LAN_ENGLISH		= 0,
	NIN_LAN_GERMAN		= 1,
	NIN_LAN_FRENCH		= 2,
	NIN_LAN_SPANISH		= 3,
	NIN_LAN_ITALIAN		= 4,
	NIN_LAN_DUTCH		= 5,

/* Auto will use English for E/P region codes and 
   only other languages when these region codes are used: D/F/S/I/J  */

	NIN_LAN_AUTO		= -1, 
};

enum VideoModes
{
	GCVideoModeNone		= 0,
	GCVideoModePAL60	= 1,
	GCVideoModeNTSC		= 2,
	GCVideoModePROG		= 3,
};

#endif