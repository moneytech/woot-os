OUTFILE = usertest

OBJS = usertest.o

TOP_DIR ?= ../..
BIN_FILE = $(TOP_DIR)/root/bin/$(OUTFILE)

COMMON_FLAGS = -ggdb -m32 -fno-stack-protector -nostdinc -nostdlib -I$(TOP_DIR)/common/include -I$(TOP_DIR)/user/include
CFLAGS += $(COMMON_FLAGS)
CXXFLAGS += $(COMMON_FLAGS)
LDFLAGS += -melf_i386 -static $(TOP_DIR)/user/lib/crt1.o -L$(TOP_DIR)/user/lib
LIBS = -lc

all: $(OUTFILE)

$(OUTFILE): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(OUTFILE) $(LIBS)

install: $(BIN_FILE)

$(BIN_FILE): $(OUTFILE)
	cp $? $@

clean:
	rm -rf $(OBJS) $(OUTFILE)

.PHONY: clean install

