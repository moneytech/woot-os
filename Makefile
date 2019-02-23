LOOP_DEVICE = /dev/loop2
TOP_DIR = $(shell realpath .)
SYSTEM_DIR = root/system
MOUNTPOINT = /mnt

CC ?= clang
CXX ?= clang++
LD ?= ld
AS ?= as
AR ?= ar
ASM ?= yasm

THREADS = 8

SUBDIRS = kernel user

export TOP_DIR

all: subdirs root hdd-image

subdirs:
	for dir in $(SUBDIRS); do \
		$(MAKE) -j$(THREADS) -C $$dir; \
	done

clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done

user:
	$(MAKE) -C user

install: root

root: root/system root/boot/grub root/bin root/lib root/boot/grub/grub.cfg
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir install; \
	done

root/system:
	mkdir -p root/system

root/boot/grub:
	mkdir -p root/boot/grub

root/bin:
	mkdir -p root/bin

root/lib:
	mkdir -p root/lib

root/boot/grub/grub.cfg: kernel/boot/grub/grub.cfg
	cp $? $@
	
hdd.img: hdd-empty-ext2.img.gz
	gunzip -c $? > $@
	$(MAKE) try-mount
	grub-install --boot-directory=$(MOUNTPOINT)/boot $(LOOP_DEVICE)
	$(MAKE) try-umount

clean-hdd-image:
	rm -f hdd.img

hdd-image: hdd.img root
	$(MAKE) try-mount
	-cp -r ./root/* $(MOUNTPOINT)
	$(MAKE) try-umount

try-mount:
	sudo losetup -P $(LOOP_DEVICE) hdd.img
	sudo mount $(LOOP_DEVICE)p1 $(MOUNTPOINT)

try-umount:
	-sudo umount $(MOUNTPOINT)
	-sudo losetup -D $(LOOP_DEVICE)

distclean: clean clean-hdd-image
	rm -rf root

.PHONY: clean distclean root kernel user

