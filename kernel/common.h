#ifndef __COMMON_H__
#define __COMMON_H__

#define SEEK_CUR    1
#define SEEK_END    2
#define SEEK_SET    0

extern void *memset8( void *dst, int x, size_t len );
extern void *memset16( void *dst, int x, size_t len );
extern void *memset32( void *dst, int x, size_t len );
extern void *memset8( void *dst, int x, size_t len );

void BootStatus( s32 Value );
void udelay(int us);
void Asciify( char *str );
unsigned int atox( char *String );
void Shutdown( void );
void W32( u32 Address, u32 Data );
u32 R32(u32 Address);

#endif
