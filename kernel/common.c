#include "string.h"
#include "syscalls.h"
#include "global.h"
#include "DI.h"
#include "EXI.h"
#include "Config.h"

#ifndef DEBUG
#define dbgprintf(...)
#else
extern int dbgprintf( const char *fmt, ...);
#endif

void BootStatus( s32 Value )
{
	memset32( (void*)0x10004100, Value, 0x20 );
	sync_after_write( (void*)0x10004100, 0x20 );
}
/*
	Since Starlet can only access MEM1 via 32byte write and 8/16 writes
	cause unpredictable results we this code is needed.

	This automatically detects the misalignment and writes the value
	via two 32bit writes
*/
void W32( u32 Address, u32 Data )
{
	if( Address & 3 )
	{		
		u32 valA = read32( Address & (~3) );
		u32 valB = read32( (Address+3) & (~3) );

		u32 Off = Address & 3;
	
		valA = valA & (0xFFFFFFFF<<((4-Off)*8));
		valB = valB & (0xFFFFFFFF>>(Off*8));
	
		valA |= Data >> (Off*8) ;
		valB |= Data << ((4-Off)*8) ;
		
		write32( Address & (~3), valA );
	//	dbgprintf("[%08X] %08X\n", Address & (~3), valA );
		write32( (Address+3) & (~3), valB );
	//	dbgprintf("[%08X] %08X\n", (Address+3) & (~3), valB );

	} else {
		write32( Address, Data );
	}
}
u32 R32(u32 Address)
{
	u32 Data;
	if (Address & 3)
	{
		u32 valA = read32(Address & (~3));
		u32 valB = read32((Address + 3) & (~3));
		
		u32 Off = Address & 3;
		
		valA = valA << ((4 - Off) * 8);
		valB = valB >> (Off * 8);
		
		Data = valA | valB;
	} else {
		Data = read32(Address);
	}
	return Data;
}
void udelay(int us)
{
	u8 heap[0x10];
	u32 msg;
	s32 mqueue = -1;
	s32 timer = -1;

	mqueue = mqueue_create(heap, 1);
	if(mqueue < 0)
		goto out;
	timer = TimerCreate(us, 0, mqueue, 0xbabababa);
	if(timer < 0)
		goto out;
	mqueue_recv(mqueue, &msg, 0);
	
out:
	if(timer > 0)
		TimerDestroy(timer);
	if(mqueue > 0)
		mqueue_destroy(mqueue);
}
void Shutdown( void )
{
	if( ConfigGetConfig(NIN_CFG_MEMCARDEMU) )
		EXIShutdown();

#ifdef DEBUG
	int i;
	//if( ConfigGetConfig(NIN_CFG_MEMEMU) )
	//{
	//	for( i = 0; i < 0x20; i+=4 )
	//		dbgprintf("0x%08X:0x%08X\t0x%08X\n", i, read32( EXI_BASE + i ), read32( EXI_SHADOW + i ) );
	//	dbgprintf("\n");
	//}

	for( i = 0; i < 0x30; i+=4 )
		dbgprintf("0x%08X:0x%08X\t0x%08X\n", i, read32( DI_BASE + i ), read32( DI_SHADOW + i ) );
	dbgprintf("\n");

	for( i = 0; i < 0x30; i+=4 )
		dbgprintf("0x%08X:0x%08X\t0x%08X\n", 0x0D806000 + i, read32( 0x0D806000 + i ), read32( 0x0D006000 + i ) );
#endif

	if( IsWiiU )
	{
		write32( 0x0D8005E0, 0xFFFFFFFE );

	} else {		
		set32( HW_GPIO_ENABLE, GPIO_POWER );
		set32( HW_GPIO_OUT, GPIO_POWER );
	}

	while(1);
}
void Asciify( char *str )
{
	int i=0;
	int length = strlen(str);
	for( i=0; i < length; i++ )
		if( str[i] < 0x20 || str[i] > 0x7F )
			str[i] = '_';
}
unsigned int atox( char *String )
{
	u32 val=1;
	u32 len=0;
	u32 i;

	while(val)
	{
		switch(String[len])
		{
			case 0x0a:
			case 0x0d:
			case 0x00:
			case ',':
				val = 0;
				len--;
				break;
		}
		len++;
	}

	for( i=0; i < len; ++i )
	{
		if( String[i] >= '0' && String[i] <='9' )
		{
			val |= (String[i]-'0') << (((len-1)-i) * 4);

		} else if( String[i] >= 'A' && String[i] <='Z' ) {

			val |= (String[i]-'7') << (((len-1)-i) * 4);

		} else if( String[i] >= 'a' && String[i] <='z' ) {

			val |= (String[i]-'W') << (((len-1)-i) * 4);
		}
	}

	return val;
}
