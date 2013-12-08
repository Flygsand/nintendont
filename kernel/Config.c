#include "Config.h"
#include "ff.h"

NIN_CFG *ncfg;
bool iswiiu = 0;

void ConfigInit( void )
{
	FIL cfg;
	u32 read;

#ifdef DEBUG
	dbgprintf("CFGInit()\n");
#endif

	ncfg = (NIN_CFG*)malloc(sizeof(NIN_CFG));

	if( f_open( &cfg, "/nincfg.bin", FA_OPEN_EXISTING|FA_READ ) != FR_OK )
	{
#ifdef DEBUG
		dbgprintf("CFG:Failed to open config\n");
#endif
		Shutdown();
	}

	f_read( &cfg, ncfg, sizeof(NIN_CFG), &read );
	f_close( &cfg );

	if( read != sizeof(NIN_CFG) )
	{
#ifdef DEBUG
		dbgprintf("CFG:Failed to read config\n");
#endif
		Shutdown();
	}

	if( (*(u32*)0x0d8005A0 >> 16 ) == 0xCAFE )
	{
		iswiiu = 1;
	} else {
		iswiiu = 0;
	}

	if( IsWiiU )
		ncfg->Config |= NIN_CFG_HID;

	if( (read32(0) >> 8) == 0x47504F )	// PSO 1&2 disable cheats/debugging
	{
		ncfg->Config &= ~(NIN_CFG_CHEATS|NIN_CFG_DEBUGGER|NIN_CFG_DEBUGWAIT);
	}

}
char *ConfigGetGamePath( void )
{
	return ncfg->GamePath;
}
char *ConfigGetCheatPath( void )
{
	return ncfg->CheatPath;
}
bool ConfigGetConfig( u32 Config )
{
	return !!(ncfg->Config&Config);
}
u32 ConfigGetVideoMode( void )
{
	return ncfg->VideoMode;
}
u32 ConfigGetLanguage( void )
{
	return ncfg->Language;
}
//bool IsWiiU
//{
//	return iswiiu;
//}
