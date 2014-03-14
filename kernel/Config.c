#include "Config.h"
#include "ff.h"

NIN_CFG *ncfg;

#ifndef DEBUG
#define dbgprintf(...)
#else
extern int dbgprintf( const char *fmt, ...);
#endif

void ConfigInit( void )
{
	FIL cfg;
	u32 read;

	dbgprintf("CFGInit()\n");

	ncfg = (NIN_CFG*)malloc(sizeof(NIN_CFG));

	if( f_open( &cfg, "/nincfg.bin", FA_OPEN_EXISTING|FA_READ ) != FR_OK )
	{
		dbgprintf("CFG:Failed to open config\n");
		Shutdown();
	}

	f_read( &cfg, ncfg, sizeof(NIN_CFG), &read );
	f_close( &cfg );

	if( read != sizeof(NIN_CFG) )
	{
		dbgprintf("CFG:Failed to read config\n");
		Shutdown();
	}

	if( IsWiiU )
		ncfg->Config |= NIN_CFG_HID;

	if( (read32(0) >> 8) == 0x47504F )	// PSO 1&2 disable cheats/debugging
	{
		ncfg->Config &= ~(NIN_CFG_CHEATS|NIN_CFG_DEBUGGER|NIN_CFG_DEBUGWAIT);
	}

}
inline char *ConfigGetGamePath( void )
{
	return ncfg->GamePath;
}
inline char *ConfigGetCheatPath( void )
{
	return ncfg->CheatPath;
}
inline bool ConfigGetConfig( u32 Config )
{
	return !!(ncfg->Config&Config);
}
inline u32 ConfigGetVideoMode( void )
{
	return ncfg->VideoMode;
}
inline u32 ConfigGetLanguage( void )
{
	return ncfg->Language;
}
