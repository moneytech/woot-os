SUBDIRS = musl usertest
THREADS ?= 8

all: subdirs

subdirs:
	for dir in $(SUBDIRS); do \
		$(MAKE) -j$(THREADS) -C $$dir; \
	done

install:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir install; \
	done

clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done

.PHONY: clean install subdirs

