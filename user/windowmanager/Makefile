OUTFILE = windowmanager

OBJS = wm.o

CC ?= clang
CXX ?= clang++

TOP_DIR ?= ../..
BIN_FILE = $(TOP_DIR)/root/bin/$(OUTFILE)
START_FILE = $(TOP_DIR)/user/lib/crt1.o
LIBC_FILE = $(TOP_DIR)/user/lib/libc.so

COMMON_FLAGS = -fPIC -ggdb -m32 -fno-stack-protector -nostdinc -nostdlib -I$(TOP_DIR)/common/include -I$(TOP_DIR)/user/include
CFLAGS += $(COMMON_FLAGS)
CXXFLAGS += $(COMMON_FLAGS)
LDFLAGS += -m32 -nostartfiles -nostdlib -L$(TOP_DIR)/user/lib

all: $(OUTFILE)

$(OUTFILE): $(OBJS) $(START_FILE) $(LIBC_FILE)
	$(CC) $(LDFLAGS) $(START_FILE) $(OBJS) -o $(OUTFILE) -lc -lwoot -lpng16 -lgcc

install: $(BIN_FILE)

$(BIN_FILE): $(OUTFILE)
	cp $? $@

clean:
	rm -rf $(OBJS) $(OUTFILE)

.PHONY: clean install

