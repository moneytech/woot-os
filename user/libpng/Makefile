TOP_DIR = $(shell realpath ../..)

MAIN_FILES = $(TOP_DIR)/user/lib/libpng16.a $(TOP_DIR)/user/lib/libpng16.so

all: configure
	$(MAKE) -C build

install: $(MAIN_FILES)

$(MAIN_FILES):
	$(MAKE) -C build install

configure: build/config.status

reconfigure:
	-rm build/config.status
	$(MAKE) configure

build/config.status:
	mkdir -p build
	cd build && \
	rm -rf ./* && \
	CC="clang -m32 -nostdinc -I$(TOP_DIR)/user/include -I$(TOP_DIR)/common/include" \
	LDFLAGS="-nostdlib -L$(TOP_DIR)/user/lib" \
	LIBS="-lc" \
	../libpng-1.6.36/configure --prefix="$(TOP_DIR)/user" --host=i686-woot

clean:
	$(MAKE) -C build clean

.PHONY: clean install configure

