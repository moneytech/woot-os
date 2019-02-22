CC = clang -m32
CFLAGS = -ggdb -fno-stack-protector -D__WOOT__

PREFIX = $(shell realpath ..)
CONFIG_LOG = build/config.log

all: configure
	$(MAKE) -C build

clean:
	$(MAKE) -C build clean

install:
	$(MAKE) -C build install

configure: $(CONFIG_LOG)

reconfigure:
	rm -rf build
	$(MAKE) configure

$(CONFIG_LOG):
	mkdir -p build && cd build && ../musl-1.1.21/configure --prefix=$(PREFIX)

.PHONY: clean install configure reconfigure

