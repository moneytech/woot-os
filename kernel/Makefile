OUTFILE = kernel

# boot/boot.o should be the first file passed to linker
OBJS = boot/boot.o \
	ahci/ahcidrive.o \
	bitmap.o \
	callbackstream.o \
	character.o \
	cmos.o \
	cpu.o \
	cxa.o \
	drive.o \
	elf.o \
	gdt.o \
	heap.o \
	ide/idedrive.o \
	ints.o \
	irqs.o \
	isrs.o \
	main.o \
	memory.o \
	memorystream.o \
	misc.o \
	mutex.o \
	new.o \
	objectqueue.o \
	objecttree.o \
	paging.o \
	pci.o \
	process.o \
	random.o \
	semaphore.o \
	softdiv.o \
	stream.o \
	string.o \
	stringbuilder.o \
	thread.o \
	time.o \
	tokenizer.o \
	uuid.o \
	v86/v86.o \
	v86/v86asm.o \

DEPS = link.ld

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

