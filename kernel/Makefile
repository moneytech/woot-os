OUTFILE = kernel

# boot/boot.o should be the first file passed to linker
OBJS = boot/boot.o \
	ahci/ahcidrive.o \
	bitmap.o \
	bufferedvolume.o \
	callbackstream.o \
	character.o \
	cmos.o \
	cpu.o \
	cxa.o \
    debug.o \
    dentry.o \
    directoryentry.o \
	drive.o \
	elf.o \
    ext2/ext2.o \
    file.o \
    filestream.o \
	filesystem.o \
	filesystemtype.o \
	gdt.o \
	heap.o \
	ide/idedrive.o \
    inode.o \
    inputdevice.o \
	ints.o \
	irqs.o \
	isrs.o \
	main.o \
	memory.o \
	memorystream.o \
	misc.o \
	module.o \
	mutex.o \
	new.o \
	objectqueue.o \
	objecttree.o \
	paging.o \
	partvolume.o \
	pci.o \
	process.o \
	random.o \
	semaphore.o \
	softdiv.o \
	stream.o \
	string.o \
	stringbuilder.o \
    syscalls.o \
	thread.o \
	time.o \
	tokenizer.o \
	uuid.o \
	v86/v86.o \
	v86/v86asm.o \
	volume.o \
	volumetype.o

DEPS = link.ld

MODULES = ps2input

TOP_DIR ?= ..
KERNEL_FILE = $(TOP_DIR)/root/system/kernel
MODULELIST_FILE = $(TOP_DIR)/root/system/modulelist

CC ?= clang
CXX ?= clang++
LD ?= ld
ASM ?= yasm

ASMFLAGS ?= -f elf32 -g dwarf2 -w

COMPILE_FLAGS = -m32 -O0 -nostdinc -nostdlib -fno-stack-protector \
    -fshort-wchar -Ulinux -Uunix -D__WOOT__=1 -ggdb \
	-I$(TOP_DIR)/kernel/include -I$(TOP_DIR)/common/include

CFLAGS += $(COMPILE_FLAGS)
CXXFLAGS += $(COMPILE_FLAGS) -fno-exceptions -fno-rtti
LDFLAGS += -melf_i386 -shared -Bsymbolic
ASFLAGS +=
ASMFLAGS +=

export TOP_DIR CC CXX LD ASM ASMFLAGS CFLAGS CXXFLAGS LDFLAGS ASFLAGS ASMFLAGS

KLDFLAGS = $(LDFLAGS) -Tlink.ld --unresolved-symbols=report-all

all: $(DEPS) $(OUTFILE) $(MODULES)

$(OUTFILE): $(OBJS)
	$(LD) $(KLDFLAGS) $(OBJS) -o $(OUTFILE)

%.o: %.asm
	$(ASM) $(ASMFLAGS) -o $@ $<

clean: clean-modules
	rm -rf $(OBJS) $(OUTFILE)

install: $(KERNEL_FILE) $(MODULELIST_FILE) $(MODULES) install-modules

todo:
	grep -rnw . --color=auto -E -e '(;|//)[ \t]*TODO:'

fixme:
	grep -rnw . --color=auto -E -e '(;|//)[ \t]*FIXME:'

$(KERNEL_FILE): $(OUTFILE)
	cp $(OUTFILE) $(KERNEL_FILE)

$(MODULELIST_FILE): modulelist
	cp modulelist $(MODULELIST_FILE)

$(MODULES):
	for dir in $(MODULES); do \
		$(MAKE) -j$(THREADS) -C $$dir; \
	done

install-modules:
	for dir in $(MODULES); do \
		$(MAKE) -j$(THREADS) -C $$dir install; \
	done

clean-modules:
	for dir in $(MODULES); do \
		$(MAKE) -j$(THREADS) -C $$dir clean; \
	done

.PHONY: clean install todo fixme clean-modules install-modules $(MODULES)

