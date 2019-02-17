OUTFILE = ps2kbd
MODULE_FILE = $(TOP_DIR)/root/system/$(OUTFILE)

OBJS = module.o

LDFLAGS += -T$(TOP_DIR)/kernel/module.ld -Ttext=0xC8000000

all: $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(OUTFILE)

clean:
	rm -rf $(OBJS) $(OUTFILE)

install: $(MODULE_FILE)

$(MODULE_FILE): $(OUTFILE)
	cp $? $@

.PHONY: clean install

