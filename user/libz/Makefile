TOP_DIR = $(shell realpath ../..)

MAIN_FILES = $(TOP_DIR)/user/lib/libz.a $(TOP_DIR)/user/lib/libz.so

all: configure
	$(MAKE) -C build

install: $(MAIN_FILES)

$(MAIN_FILES):
	$(MAKE) -C build install

configure: build/configure.log

reconfigure:
	-rm build/configure.log
	$(MAKE) configure

build/configure.log:
	mkdir -p build
	cd build && \
	rm -rf ./* && \
	SONAME=libz.so \
	CC="clang -m32 -nostdinc -I$(TOP_DIR)/user/include -I$(TOP_DIR)/common/include" \
	LDSHAREDLIBC="-nostdlib -Wl,-L$(TOP_DIR)/user/lib -lgcc" \
	../zlib-1.2.11/configure --prefix="$(TOP_DIR)/user"

clean:
	-rm $(TOP_DIR)/user/lib/libz.so
	$(MAKE) -c build clean

.PHONY: clean install configure

