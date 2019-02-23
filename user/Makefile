SUBDIRS = usertest
THREADS ?= 8

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

install:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir install; \
	done

clean: clean-musl
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done

.PHONY: clean install subdirs musl clean-musl

