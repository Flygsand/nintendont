OUTPUT_FORMAT("elf32-bigarm")
OUTPUT_ARCH(arm)
EXTERN(_start)
ENTRY(_start)

__stack_size = 32;

MEMORY {

	code		: ORIGIN = 0x20100000,	LENGTH = 0xD000
	data		: ORIGIN = 0x2010D000,	LENGTH = 0x1000
	bss			: ORIGIN = 0x2010E000,	LENGTH = 0x46A8
}

PHDRS
{
	code PT_LOAD AT ( 0x139F0000 );
	data PT_LOAD AT ( 0x139FD000 );
	bss  PT_LOAD AT ( 0x139FE000 );
}

SECTIONS
{
	.init :
	{
		*(.init)
		. = ALIGN(4);
	} >code :code

	.text :
	{
		*(.text*)
		*(.text.*)
		*(.gnu.warning)
		*(.gnu.linkonce.t*)
		*(.glue_7)
		*(.glue_7t)
		. = ALIGN(4);
	} >code :code

	.rodata :
	{
		*(.rodata)
		*all.rodata*(*)
		*(.roda)
		*(.rodata.*)
		*(.gnu.linkonce.r*)
		. = ALIGN(4);
	} >code :code

	.data :
	{
		*(.data)
		*(.data.*)
		*(.gnu.linkonce.d*)
		. = ALIGN(4);
	} >data :data

	.bss :
	{
		__bss_start = . ;
		*(.dynbss)
		*(.gnu.linkonce.b*)
		*(.bss*)
		*(COMMON)
		. = ALIGN(4);
		__bss_end = . ;
	} >bss :bss

	.stack :
	{
		__stack_end = .;
		. = . +__stack_size;
		. = ALIGN(64);
		__stack_addr = .;
	} >bss :bss

}

