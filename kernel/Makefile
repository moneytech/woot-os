OUTFILE = kernel

# boot/boot.o should be the first file passed to linker
OBJS = boot/boot.o \
	bitmap.o \
	cpu.o \
	gdt.o \
	main.o \
	memory.o \
	new.o \
	paging.o

DEPS = link.ld \
	../common/include/sysdefs.h \
	include/bitmap.hpp \
	include/cpu.hpp \
	include/gdt.hpp \
	include/memory.hpp \
	include/misc.hpp \
	include/new.hpp \
	include/paging.hpp \
	include/types.hpp

TOP_DIR ?= ..
KERNEL_FILE = $(TOP_DIR)/root/system/kernel

CC ?= clang
CXX ?= clang++
LD ?= ld
ASM ?= yasm

ASMFLAGS ?= -f elf32 -g dwarf2 -w

COMPILE_FLAGS = -m32 -nostdinc -nostdlib -fno-stack-protector \
	-Ulinux -Uunix -D__WOOT__=1 -ggdb \
	-I$(TOP_DIR)/kernel/include -I$(TOP_DIR)/common/include

CFLAGS += $(COMPILE_FLAGS)
CXXFLAGS += $(COMPILE_FLAGS) -fno-exceptions -fno-rtti
LDFLAGS += -melf_i386 -Tlink.ld
ASFLAGS +=
ASMFLAGS +=

all: $(DEPS) $(OUTFILE)

$(OUTFILE): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(OUTFILE)

%.o: %.asm
	$(ASM) $(ASMFLAGS) -o $@ $<

clean:
	rm -rf $(OBJS) $(OUTFILE)

install: $(KERNEL_FILE)

$(KERNEL_FILE): $(OUTFILE)
	cp $(OUTFILE) $(KERNEL_FILE)

.PHONY: clean install

