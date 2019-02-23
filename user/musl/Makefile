TOP_DIR ?= $(shell realpath ../..)
PREFIX = $(TOP_DIR)/user
CONFIG_FILE = build/config.mak
THREADS = 8

CC = clang
CFLAGS = -m32 -O0 -D__WOOT__ -ggdb -I$(TOP_DIR)
ASFLAGS = --32
LDFLAGS = -Wl,-T$(TOP_DIR)/user/shared.ld
AR = ar
RANLIB = ranlib

LIBCC = 
#$(PREFIX)/../libclang_rt.builtins-i386.a

export CC AR RANLIB CFLAGS ASFLAGS LDFLAGS LIBCC

all: configure
	$(MAKE) -j8 -C build

clean:
	$(MAKE) -C build clean

install:
	$(MAKE) -C build install

uninstall:
	$(MAKE) -C build uninstall

configure: $(CONFIG_FILE)

reconfigure:
	rm -rf build
	$(MAKE) configure

$(CONFIG_FILE):
	mkdir -p build && cd build && ../musl-1.1.21/configure --prefix=$(PREFIX) --target=woot

.PHONY: clean install uninstall configure reconfigure

