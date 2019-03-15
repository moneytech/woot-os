SUBDIRS = init inputhandler usertest windowmanager
THREADS ?= 8

TOP_DIR ?= $(shell realpath ..)

all: musl libwoot subdirs

subdirs: musl
	for dir in $(SUBDIRS); do \
		$(MAKE) -j$(THREADS) -C $$dir; \
	done

musl:
	$(MAKE) -C musl all install

libwoot:
	$(MAKE) -C libwoot all install

clean-musl:
	$(MAKE) -C musl clean
	
clean-libwoot:
	$(MAKE) -C libwoot clean

install:
	mkdir -p $(TOP_DIR)/root/etc
	echo "/lib" > $(TOP_DIR)/root/etc/ld-musl-i386.path
	cp lib/*.so $(TOP_DIR)/root/lib/
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir install; \
	done

clean: clean-musl clean-libwoot
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done

.PHONY: clean install subdirs musl clean-musl install-musl libwoot clean-libwoot

