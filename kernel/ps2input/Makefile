OUTFILE = ps2input
MODULE_FILE = $(TOP_DIR)/root/system/$(OUTFILE)

OBJS = module.o ps2.o ps2kbd.o ps2mouse.o

TOP_DIR ?= $(shell realpath ../..)
CUR_DIR = $(shell realpath .)

CFLAGS += -I$(CUR_DIR)
CXXFLAGS += -I$(CUR_DIR)
LDFLAGS += -T$(TOP_DIR)/kernel/module.ld -Ttext=0xC8001000

all: $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(OUTFILE)

clean:
	rm -rf $(OBJS) $(OUTFILE)

install: $(MODULE_FILE)

$(MODULE_FILE): $(OUTFILE)
	cp $? $@

.PHONY: clean install

