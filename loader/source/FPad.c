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
#include "FPad.h"

u32 WPAD_Pressed;
u32 PAD_Pressed;
s8	PAD_Stick_Y;
s8	PAD_Stick_X;
u32 SLock;

u64 DelayX;
u32 SpeedX;

u64 DelayY;

#define DELAY_START	900
#define DELAY_STEP	100
#define DELAY_STOP	100

void FPAD_Init( void )
{
	PAD_Init();
	WPAD_Init();

	WPAD_Pressed = 0;
	PAD_Pressed = 0;
}
void FPAD_Update( void )
{
	WPAD_ScanPads();
	PAD_ScanPads();
	
	WPAD_Pressed = WPAD_ButtonsDown(0) | WPAD_ButtonsDown(1) | WPAD_ButtonsDown(2) | WPAD_ButtonsDown(3);
	WPAD_Pressed |= WPAD_ButtonsHeld(0) | WPAD_ButtonsHeld(1) | WPAD_ButtonsHeld(2) | WPAD_ButtonsHeld(3);
	PAD_Pressed  = PAD_ButtonsDown(0) | PAD_ButtonsDown(1) | PAD_ButtonsDown(2) | PAD_ButtonsDown(3);
	PAD_Pressed  |= PAD_ButtonsHeld(0) | PAD_ButtonsHeld(1) | PAD_ButtonsHeld(2) | PAD_ButtonsHeld(3);
	PAD_Stick_Y	 = PAD_StickY(0) | PAD_StickY(1) | PAD_StickY(2) | PAD_StickY(3);
	PAD_Stick_X	 = PAD_StickX(0) | PAD_StickX(1) | PAD_StickX(2) | PAD_StickX(3);
		
	if( WPAD_Pressed == 0 && PAD_Pressed == 0 && ( PAD_Stick_Y < 25 && PAD_Stick_Y > -25 )  && ( PAD_Stick_X < 25 && PAD_Stick_X > -25 ) )
	{
		SLock = 0;
		SpeedX= DELAY_START;
	}
}
bool FPAD_Up( u32 ILock )
{
	//if( diff_msec( DelayX, gettime() ) < SpeedX )
	//{
		if( !ILock && SLock )
			return 0;
	//}

	//if( SpeedX > DELAY_STOP )
	//	SpeedX -= DELAY_STEP;

	if( WPAD_Pressed & (WPAD_BUTTON_UP|WPAD_CLASSIC_BUTTON_UP) )
	{
		SLock = 1;
//		DelayX = gettime();
		return 1;
	}
	if( PAD_Pressed & PAD_BUTTON_UP )
	{
		SLock = 1;
//		DelayX = gettime();
		return 1;
	}
	if( PAD_Stick_Y > 30 )
	{
		SLock = 1;
//		DelayX = gettime();
		return 1;
	}

	return 0;
}

bool FPAD_Down( u32 ILock )
{
	//if( diff_msec( DelayX, gettime() ) < SpeedX )
	//{
		if( !ILock && SLock )
			return 0;
	//}

	//if( SpeedX > DELAY_STOP )
	//	SpeedX -= DELAY_STEP;

	if( WPAD_Pressed & (WPAD_BUTTON_DOWN|WPAD_CLASSIC_BUTTON_DOWN) )
	{
		SLock = 1;
//		DelayX = gettime();
		return 1;
	}
	if( PAD_Pressed & PAD_BUTTON_DOWN )
	{
		SLock = 1;
//		DelayX = gettime();
		return 1;
	}
	if( PAD_Stick_Y < -30 )
	{
		SLock = 1;
//		DelayX = gettime();
		return 1;
	}

	return 0;
}

bool FPAD_Left( u32 ILock )
{
	if( !ILock && SLock )
		return 0;

	if( WPAD_Pressed & (WPAD_BUTTON_LEFT|WPAD_CLASSIC_BUTTON_LEFT) )
	{
		SLock = 1;
		return 1;
	}
	if( PAD_Pressed & PAD_BUTTON_LEFT )
	{
		SLock = 1;
		return 1;
	}
	if( PAD_Stick_X < -30 )
	{
		SLock = 1;
		return 1;
	}

	return 0;
}
bool FPAD_Right( u32 ILock )
{
	if( !ILock && SLock )
		return 0;

	if( WPAD_Pressed & (WPAD_BUTTON_RIGHT|WPAD_CLASSIC_BUTTON_RIGHT) )
	{
		SLock = 1;
		return 1;
	}
	if( PAD_Pressed & PAD_BUTTON_RIGHT )
	{
		SLock = 1;
		return 1;
	}
	if( PAD_Stick_X > 30 )
	{
		SLock = 1;
		return 1;
	}

	return 0;
}
bool FPAD_OK( u32 ILock )
{
	if( !ILock && SLock )
		return 0;

	if( WPAD_Pressed & (WPAD_BUTTON_A|WPAD_CLASSIC_BUTTON_A) )
	{
		SLock = 1;
		return 1;
	}
	if( PAD_Pressed & PAD_BUTTON_A )
	{
		SLock = 1;
		return 1;
	}

	return 0;
}

bool FPAD_X( u32 ILock )
{
	if( !ILock && SLock )
		return 0;

	if( WPAD_Pressed & (WPAD_BUTTON_1|WPAD_CLASSIC_BUTTON_X) )
	{
		SLock = 1;
		return 1;
	}
	if( PAD_Pressed & PAD_BUTTON_X )
	{
		SLock = 1;
		return 1;
	}

	return 0;
}

bool FPAD_Cancel( u32 ILock )
{
	if( !ILock && SLock )
		return 0;

	if( WPAD_Pressed & (WPAD_BUTTON_B|WPAD_CLASSIC_BUTTON_B) )
	{
		SLock = 1;
		return 1;
	}
	if( PAD_Pressed & PAD_BUTTON_B )
	{
		SLock = 1;
		return 1;
	}

	return 0;
}

bool FPAD_Start( u32 ILock )
{
	if( !ILock && SLock )
		return 0;

	if( WPAD_Pressed & (WPAD_BUTTON_HOME|WPAD_CLASSIC_BUTTON_HOME) )
	{
		SLock = 1;
		return 1;
	}
	if( PAD_Pressed & PAD_BUTTON_START )
	{
		SLock = 1;
		return 1;
	}

	return 0;
}
