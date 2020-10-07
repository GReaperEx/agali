INCD := include
SRCD := src
OBJD := obj

$(shell mkdir -p $(OBJD) >/dev/null)

SRCS := $(wildcard $(SRCD)/*.c) $(wildcard $(SRCD)/*.asm)
OBJS := $(patsubst $(SRCD)/%.asm,$(OBJD)/%.o,$(patsubst $(SRCD)/%.c,$(OBJD)/%.o,$(SRCS)))
DEPS := $(OBJS:.o=.d)

CFLAGS ?= -O2 -Wall -Wextra
CFLAGS := $(CFLAGS) -I$(INCD) -MMD -MP -ffreestanding -fno-builtin -mno-red-zone -nostdinc -nostdlib -nostartfiles -nodefaultlibs -mno-ms-bitfields -m64 -mcmodel=medium -fPIC

LD ?= ld
CC ?= gcc

kernel.sys : $(OBJS)
	@echo Linking: $@
	@$(LD) $(OBJS) -o kernel.sys --nmagic -T linker.ld

$(OBJD)/%.o : $(SRCD)/%.c
	@echo Compiling: $(<F)
	@$(CC) $(CFLAGS) -c -o $@ $<

$(OBJD)/%.o : $(SRCD)/%.asm
	@echo Compiling: $(<F)
	@nasm -fcoff -o $@ $<

.PHONY : clean
clean :
	rm -rf kernel.sys $(OBJD)

-include $(DEPS)
