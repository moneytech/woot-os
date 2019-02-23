OUTFILE = usertest

OBJS = usertest.o

TOP_DIR ?= ../..
BIN_FILE = $(TOP_DIR)/root/bin/$(OUTFILE)

COMMON_FLAGS = -fPIC -ggdb -m32 -fno-stack-protector -nostdinc -nostdlib -I$(TOP_DIR)/common/include -I$(TOP_DIR)/user/include
CFLAGS += $(COMMON_FLAGS)
CXXFLAGS += $(COMMON_FLAGS)
#LDFLAGS += -melf_i386 -T$(TOP_DIR)/user/exec.ld -Ttext=0x02001000 -static $(TOP_DIR)/user/lib/crt1.o -L$(TOP_DIR)/user/lib
LDFLAGS += -melf_i386 -T$(TOP_DIR)/user/exec.ld -Ttext=0x02001000 -shared $(TOP_DIR)/user/lib/Scrt1.o -L$(TOP_DIR)/user/lib
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

