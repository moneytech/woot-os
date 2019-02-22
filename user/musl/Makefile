CC = clang -m32
CFLAGS = -ggdb -fno-stack-protector -D__WOOT__

PREFIX = $(shell realpath ..)
CONFIG_FILE = build/config.mak

all: configure
	$(MAKE) -C build

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
	mkdir -p build && cd build && ../musl-1.1.21/configure --prefix=$(PREFIX)

.PHONY: clean install uninstall configure reconfigure

