
DIRS=emulator display

ROOT_DIR=$(CURDIR)
	
all:
	for dir in $(DIRS); do \
		make -C $(ROOT_DIR)/$$dir; \
	done

clean:
	for dir in $(DIRS); do \
		make -C $$dir clean; \
	done

