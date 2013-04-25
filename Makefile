
DIRS=libs emulator display

ROOT_DIR=$(CURDIR)
	
all:
	for dir in $(DIRS); do \
		make -C $(ROOT_DIR)/$$dir; \
	done
	test ! -e bin && mkdir bin
	cp emulator/motonesemu bin/
	cp display/vgadisp bin/
	cp emulator/joypad/famicon-controller.jpg bin/

tag:
	find . -name "*.[ch]" | xargs ctags

clean:
	for dir in $(DIRS); do \
		make -C $$dir clean; \
	done

