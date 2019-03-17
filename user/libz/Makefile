TOP_DIR = $(shell realpath ../..)

all: configure
	$(MAKE) -C build

install:
	$(MAKE) -C build install

configure: build/configure.log

reconfigure:
	rm build/configure.log
	$(MAKE) configure

build/configure.log:
	mkdir -p build
	cd build && \
	rm -rf ./* && \
	SONAME=libz.so \
	CC="clang -m32 -nostdinc -I$(TOP_DIR)/user/include -I$(TOP_DIR)/common/include" \
	LDSHAREDLIBC="-nostdlib -Wl,-L$(TOP_DIR)/user/lib" \
	../zlib-1.2.11/configure --prefix="$(TOP_DIR)/user"

clean:
	$(MAKE) -c build clean

.PHONY: clean install configure

