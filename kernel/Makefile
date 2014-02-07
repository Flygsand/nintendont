PREFIX = $(DEVKITARM)/bin/arm-none-eabi-
CC = $(PREFIX)gcc
AS = $(PREFIX)as
LD = $(PREFIX)gcc
STRIP = $(PREFIX)strip

CFLAGS = -mbig-endian -fno-unwind-tables -fomit-frame-pointer -O2 -Wall -I. -mcpu=arm926ej-s -mthumb -fno-builtin-toupper -fno-builtin-memcpy -fno-builtin-memset  -fno-builtin-memcmp -fno-builtin-malloc -fno-builtin-free
ASFLAGS = -mbig-endian -mcpu=arm926ej-s
LDFLAGS = -nostartfiles -nodefaultlibs -mbig-endian -Wl,-T,kernel.ld,-Map,kernel.map -n
LIBS = -lgcc

TARGET = kernel.elf
OBJECTS = start.o common.o alloc.o gecko.o FST.o DI.o Patch.o StreamADPCM.o EXI.o ff.o HID.o diskio.o SDI.o Config.o utils_asm.o ES.o NAND.o main.o syscalls.o vsprintf.o string.o

.PHONY: FORCE

all: $(TARGET)

$(TARGET) : kernel.ld $(OBJECTS)
	@echo  "LD $@"
	@$(LD) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $@
	@$(STRIP) kernel.elf
	@cp kernel.elf ../loader/data/kernel.bin

%.o : %.s
	@echo  "AS $@"
	@$(CC) $(CFLAGS) -D_LANGUAGE_ASSEMBLY -c -x assembler-with-cpp -o $@ $<

%.o : %.S
	@echo  "AS $@"
	@$(CC) $(CFLAGS) -D_LANGUAGE_ASSEMBLY -c -x assembler-with-cpp -o $@ $<

%.o : %.c
	@echo  "CC $@"
	@$(CC) $(CFLAGS) -c -o $@ $<

%.d: %.c
	@echo  "DEP $@"
	@set -e; $(CC) -M $(CFLAGS) $< \
	 | sed 's?\($*\)\.o[ :]*?\1.o $@ : ?g' > $@; \
	 [ -s $@ ] || rm -f $@

%.d: %.S
	@echo "DEP $@"
	@touch $@

-include $(OBJECTS:.o=.d)

clean:
	-rm -f *.elf *.o *.bin *.d *.map
