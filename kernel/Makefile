OUTFILE = kernel
OBJS = boot/boot.o main.o
DEPS = link.ld

TOP_DIR ?= ..
KERNEL_FILE = $(TOP_DIR)/root/system/kernel

CC ?= clang
CXX ?= clang++
LD ?= ld
ASM ?= yasm

ASMFLAGS ?= -f elf32 -g dwarf2 -w

CFLAGS += -m32
CXXFLAGS += -m32
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

