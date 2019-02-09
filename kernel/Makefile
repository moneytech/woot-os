OUTFILE = kernel

OBJS = boot/boot.o # this should be the first file passed to linker
OBJS += bitmap.o main.o memory.o paging.o

DEPS = link.ld
DEPS += include/bitmap.hpp include/memory.hpp include/paging.hpp include/types.hpp

TOP_DIR ?= ..
KERNEL_FILE = $(TOP_DIR)/root/system/kernel

CC ?= clang
CXX ?= clang++
LD ?= ld
ASM ?= yasm

ASMFLAGS ?= -f elf32 -g dwarf2 -w

COMPILE_FLAGS = -m32 -I$(TOP_DIR)/kernel/include -I$(TOP_DIR)/common/include

CFLAGS += $(COMPILEF_LAGS)
CXXFLAGS += $(COMPILE_FLAGS)
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

