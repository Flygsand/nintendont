#include <asm.h>


PADRead:
	
	lis		%r4,	0xD000
	addi	%r4,	%r4,	0x3FD0

	li		%r5,	0x30
	
memcpy_pre:	
	lwz		%r0,	0(%r4)
	stw		%r0,	0(%r3)
	addic.	%r5,	%r5,	-4

memcpy:
	lwzu	%r0,	4(%r4)
	stwu	%r0,	4(%r3)
	addic.	%r5,	%r5,	-4
	bne		memcpy

	blr
