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
#ifndef _FPAD_
#define _FPAD_

#include "global.h"
#include <wiiuse\wpad.h>
#include <ogc\pad.h>

void FPAD_Init( void );
void FPAD_Update( void );

bool FPAD_Up( u32 ILock );
bool FPAD_Down( u32 ILock );
bool FPAD_Left( u32 ILock );
bool FPAD_Right( u32 ILock );
bool FPAD_OK( u32 ILock );
bool FPAD_Cancel( u32 ILock );
bool FPAD_Start( u32 ILock );

bool FPAD_X( u32 ILock );

#endif
