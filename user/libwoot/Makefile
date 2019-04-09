SHAREDLIB = libwoot.so
STATICLIB = libwoot.a

OBJS = \
	input.o \
	ipc.o \
	misc.o \
	pixmap.o \
	process.o \
	rectangle.o \
	rpc.o \
	signal.o \
	thread.o \
	video.o \
	wm.o

TOP_DIR ?= $(shell realpath ../..)
COMMON_INCLUDE_DIR ?= $(TOP_DIR)/common/include
INCLUDE_DIR ?= $(TOP_DIR)/user/include
LIB_DIR ?= $(TOP_DIR)/user/lib

CC ?= clang
AR ?= ar
LD ?= ld

CFLAGS = -m32 -nostdinc -fno-stack-protector -ggdb -I. -I$(COMMON_INCLUDE_DIR) -I$(INCLUDE_DIR)
CFLAGS += -Wno-undefined-internal
LDFLAGS = -melf_i386 -shared -Ttext=0x00400000
LIBS = -L$(TOP_DIR)/user/lib -lc -lpng16

all: $(STATICLIB) $(SHAREDLIB)

$(STATICLIB): $(OBJS)
	$(AR) rc $@ $(OBJS)

$(SHAREDLIB): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(SHAREDLIB) $(LIBS)

install:
	cp -r woot $(INCLUDE_DIR)/
	cp $(STATICLIB) $(LIB_DIR)/
	cp $(SHAREDLIB) $(LIB_DIR)/

uninstall:
	rm -rf $(INCLUDE_DIR)/woot
	rm -f $(LIB_DIR)/$(STATICLIB)
	rm -f $(LIB_DIR)/$(SHAREDLIB)

clean:
	rm -rf $(OBJS) $(STATICLIB) $(SHAREDLIB)

.PHONY: all clean install uninstall

