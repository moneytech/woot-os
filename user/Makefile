SUBDIRS = usertest
THREADS ?= 8

TOP_DIR ?= $(shell realpath ..)

SRC_LIBC_SO = lib/libc.so
DST_LIBC_SO = $(TOP_DIR)/root/lib/libc.so

all: musl subdirs

subdirs: musl
	for dir in $(SUBDIRS); do \
		$(MAKE) -j$(THREADS) -C $$dir; \
	done

musl:
	$(MAKE) -C musl
	$(MAKE) -C musl install

clean-musl:
	$(MAKE) -C musl clean

install-musl: $(DST_LIBC_SO)

$(DST_LIBC_SO): musl
	cp $(SRC_LIBC_SO) $@

install: do-install install-musl

do-install:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir install; \
	done

clean: clean-musl
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done

.PHONY: clean install do-install subdirs musl clean-musl install-musl

