OUTFILE = usertest

OBJS = usertest.o

TOP_DIR ?= ../..
BIN_FILE = $(TOP_DIR)/root/bin/$(OUTFILE)

CC ?= clang
CXX ?= clang++

COMMON_FLAGS = -ggdb -m32 -fno-stack-protector -I$(TOP_DIR)/common/include
CFLAGS += $(COMMON_FLAGS)
CXXFLAGS += $(COMMON_FLAGS)
LDFLAGS += -melf_i386

all: $(OUTFILE)

$(OUTFILE): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(OUTFILE)

install: $(BIN_FILE)

$(BIN_FILE): $(OUTFILE)
	cp $? $@

clean:
	rm -rf $(OBJS) $(OUTFILE)

.PHONY: clean install

