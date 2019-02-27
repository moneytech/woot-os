OUTFILE = usertest

OBJS = usertest.o

CC ?= clang
CXX ?= clang++

TOP_DIR ?= ../..
BIN_FILE = $(TOP_DIR)/root/bin/$(OUTFILE)
START_FILE = $(TOP_DIR)/user/lib/Scrt1.o
LIBC_FILE = $(TOP_DIR)/user/lib/libc.so

COMMON_FLAGS = -fPIC -ggdb -m32 -fno-stack-protector -nostdinc -nostdlib -I$(TOP_DIR)/common/include -I$(TOP_DIR)/user/include
CFLAGS += $(COMMON_FLAGS)
CXXFLAGS += $(COMMON_FLAGS)
#LDFLAGS += -melf_i386 -T$(TOP_DIR)/user/exec.ld -L$(TOP_DIR)/user/lib
LDFLAGS += -melf_i386 -T$(TOP_DIR)/user/exec.ld -shared -L$(TOP_DIR)/user/lib

all: $(OUTFILE)

$(OUTFILE): $(OBJS) $(START_FILE) $(LIBC_FILE)
	$(LD) $(LDFLAGS) $(START_FILE) $(OBJS) -o $(OUTFILE) -lc

install: $(BIN_FILE)

$(BIN_FILE): $(OUTFILE)
	cp $? $@

clean:
	rm -rf $(OBJS) $(OUTFILE)

.PHONY: clean install

