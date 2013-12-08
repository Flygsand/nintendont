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
#include "exi.h"
#include "global.h"
#include <stdio.h>

u32 GeckoFound = 0;
void CheckForGecko( void )
{
	if( !IsWiiU() )
		GeckoFound = usb_isgeckoalive( 1 );

}
int gprintf( const char *str, ... )
{
	if( IsWiiU() )
	{
		char astr[4096];

		va_list ap;
		va_start(ap,str);

		vsprintf( astr, str, ap );

		va_end(ap);

		FILE *log = fopen("sd:/nloader.log", "a");
		if( log != NULL )
		{
			fprintf( log, "%s", astr );
			fclose( log );
		}

	} else {

		if(!GeckoFound)
			return 0;

		char astr[4096];

		va_list ap;
		va_start(ap,str);

		vsprintf( astr, str, ap );

		va_end(ap);

		gprint( astr );
	}

	return 1;
}
void EXISendByte( char byte )
{

loop:
	*(vu32*)EXI			= 0xD0;
	*(vu32*)(EXI+0x10)	= 0xB0000000 | (byte<<20);
	*(vu32*)(EXI+0x0C)	= 0x19;

	while( *(vu32*)(EXI+0x0C)&1 );

	u32 loop =  *(vu32*)(EXI+0x10)&0x4000000;
	
	*(vu32*)EXI	= 0;

	if( !loop )
		goto loop;

	return;
}
void gprint( char *buffer )
{
	if( IsWiiU() )
	{
		FILE *log = fopen("sd:/nloader.log", "a");
		if( log != NULL )
		{
			fprintf( log, "%s", buffer );
			fclose( log );
		}
	} else {
		int i = 0;
		while( buffer[i] != '\0' )
		{
			EXISendByte( buffer[i] );
			++i;
		}
	}
	return;
}