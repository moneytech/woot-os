LIBDIRS = musl libwoot libz
SUBDIRS = init inputhandler usertest windowmanager
THREADS ?= 8

TOP_DIR ?= $(shell realpath ..)

all: libdirs subdirs

subdirs: libdirs
	for dir in $(SUBDIRS); do \
		$(MAKE) -j$(THREADS) -C $$dir; \
	done

libdirs:
	for dir in $(LIBDIRS); do \
		$(MAKE) -j$(THREADS) -C $$dir all install; \
	done

install:
	mkdir -p $(TOP_DIR)/root/etc
	echo "/lib" > $(TOP_DIR)/root/etc/ld-musl-i386.path
	cp lib/*.so $(TOP_DIR)/root/lib/
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir install; \
	done

clean:
	for dir in $(LIBDIRS) $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done

.PHONY: clean install subdirs libdirs

