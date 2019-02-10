OUTFILE = kernel

# boot/boot.o should be the first file passed to linker
OBJS = boot/boot.o \
	bitmap.o \
	callbackstream.o \
	character.o \
	cpu.o \
	cxa.o \
	gdt.o \
	heap.o \
	ints.o \
	isrs.o \
	main.o \
	memory.o \
	misc.o \
	new.o \
	paging.o \
	random.o \
	softdiv.o \
	stream.o \
	string.o \
	uuid.o

DEPS = link.ld \
	../common/include/errno.h \
	../common/include/sysdefs.h \
	include/bitmap.hpp \
	include/callbackstream.hpp \
	include/character.hpp \
	include/cpu.hpp \
	include/debug.hpp \
	include/gdt.hpp \
	include/heap.hpp \
	include/ints.hpp \
	include/list.hpp \
	include/memory.hpp \
	include/misc.hpp \
	include/new.hpp \
	include/paging.hpp \
	include/random.hpp \
	include/stream.hpp \
	include/string.hpp \
	include/types.hpp \
	include/uuid.hpp \
	include/vararg.hpp

TOP_DIR ?= ..
KERNEL_FILE = $(TOP_DIR)/root/system/kernel

CC ?= clang
CXX ?= clang++
LD ?= ld
ASM ?= yasm

ASMFLAGS ?= -f elf32 -g dwarf2 -w

COMPILE_FLAGS = -m32 -nostdinc -nostdlib -fno-stack-protector \
	-fshort-wchar -Ulinux -Uunix -D__WOOT__=1 -ggdb \
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

todo:
	grep -rnw . --color=auto -E -e '(;|//)[ \t]*TODO:'

fixme:
	grep -rnw . --color=auto -E -e '(;|//)[ \t]*FIXME:'

$(KERNEL_FILE): $(OUTFILE)
	cp $(OUTFILE) $(KERNEL_FILE)

.PHONY: clean install todo fixme

