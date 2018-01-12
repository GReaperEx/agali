SRCD := src
OBJD := .obj

DEPFLAGS.c = -MT $@ -MMD -MP -MF $(OBJD)/$*.Td
DEPFLAGS.asm = -MT $@ -MD $(OBJD)/$*.Td
POSTCOMPILE = @mv -f $(OBJD)/$*.Td $(OBJD)/$*.d && touch $@

$(shell mkdir -p $(OBJD) >/dev/null)

SRCS := $(wildcard $(SRCD)/*.c) $(wildcard $(SRCD)/*.asm)
OBJS := $(patsubst $(SRCD)/%.asm,$(OBJD)/%.o,$(patsubst $(SRCD)/%.c,$(OBJD)/%.o,$(SRCS)))

CFLAGS = -Wall -Wextra -nostdlib -fno-builtin -nostartfiles -nodefaultlibs -O2 -mno-ms-bitfields -m64 -mcmodel=medium -fPIC

ifeq ($(ENV), windows)
  LD = i686-elf-ld
  CC = i686-elf-gcc
else
  LD = ld
  CC = gcc
endif

kernel.sys : $(OBJS)
	$(LD) $(OBJS) -o kernel.sys --nmagic -T linker.ld

$(OBJD)/%.o : $(SRCD)/%.c
$(OBJD)/%.o : $(SRCD)/%.c $(OBJD)/%.d
	$(CC) $(DEPFLAGS.c) $(CFLAGS) -c -o $@ $<
	$(POSTCOMPILE)

$(OBJD)/%.o : $(SRCD)/%.asm
$(OBJD)/%.o : $(SRCD)/%.asm $(OBJD)/%.d
	nasm $(DEPFLAGS.asm) -felf64 -o $@ $<
	$(POSTCOMPILE)

$(OBJD)/%.d: ;
.PRECIOUS: $(OBJD)/%.d

include $(wildcard $(patsubst $(SRCD)/%,$(OBJD)/%.d,$(basename $(SRCS))))

.PHONY : clean
clean :
	rm -rf kernel.sys $(OBJD)

