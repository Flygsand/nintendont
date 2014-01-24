#include "alloc.h"
#include "vsprintf.h"

inline void *malloc( u32 size )
{
	void *ptr = heap_alloc( 0, size );
	if( ptr == NULL )
	{
	//	dbgprintf("Malloc:%p Size:%08X FAILED\n", ptr, size );
		Shutdown();
	}
	return ptr;
}
inline void *malloca( u32 size, u32 align )
{
	void *ptr = heap_alloc_aligned( 0, size, align );
	if( ptr == NULL )
	{
	//	dbgprintf("Malloca:%p Size:%08X FAILED\n", ptr, size );
		Shutdown();
	}
	return ptr;
}
inline void free( void *ptr )
{
	if( ptr != NULL )
		heap_free( 0, ptr );

	//dbgprintf("Free:%p\n", ptr );

	return;
}