SRCD := src
OBJD := .obj

DEPFLAGS.c = -MT $@ -MMD -MP -MF $(OBJD)/$*.Td
DEPFLAGS.asm = -MT $@ -MD $(OBJD)/$*.Td
POSTCOMPILE = mv -f $(OBJD)/$*.Td $(OBJD)/$*.d && touch $@

$(shell mkdir -p $(OBJD) >/dev/null)

SRCS := $(wildcard $(SRCD)/*.c) $(wildcard $(SRCD)/*.asm)
OBJS := $(patsubst $(SRCD)/%.asm,$(OBJD)/%.o,$(patsubst $(SRCD)/%.c,$(OBJD)/%.o,$(SRCS)))

CFLAGS ?= -O2 -Wall -Wextra
CFLAGS := $(CFLAGS) -ffreestanding -mno-red-zone -nostdlib -nostartfiles -nodefaultlibs -mno-ms-bitfields -m64 -mcmodel=medium -fPIC

LD ?= ld
CC ?= gcc

kernel.sys : $(OBJS)
	@echo Linking: $@
	@$(LD) $(OBJS) -o kernel.sys --nmagic -T linker.ld

$(OBJD)/%.o : $(SRCD)/%.c
$(OBJD)/%.o : $(SRCD)/%.c $(OBJD)/%.d
	@echo Compiling: $<
	@$(CC) $(DEPFLAGS.c) $(CFLAGS) -c -o $@ $<
	@$(POSTCOMPILE)

$(OBJD)/%.o : $(SRCD)/%.asm
$(OBJD)/%.o : $(SRCD)/%.asm $(OBJD)/%.d
	@echo Compiling: $<
	@nasm $(DEPFLAGS.asm) -felf64 -o $@ $<
	@$(POSTCOMPILE)

$(OBJD)/%.d: ;
.PRECIOUS: $(OBJD)/%.d

include $(wildcard $(patsubst $(SRCD)/%,$(OBJD)/%.d,$(basename $(SRCS))))

.PHONY : clean
clean :
	rm -rf kernel.sys $(OBJD)

