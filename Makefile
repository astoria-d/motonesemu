
DIRS=libs emulator display

ROOT_DIR=$(CURDIR)
	
all:
	for dir in $(DIRS); do \
		make -C $(ROOT_DIR)/$$dir; \
	done
	./install.sh

tag:
	find . -name "*.[ch]" | xargs ctags

clean:
	for dir in $(DIRS); do \
		make -C $$dir clean; \
	done
	rm bin/motonesemu bin/vgadisp bin/famicon-controller.jpg

