
BIN=motonesemu

OBJS=emu-main.o clock.o bus.o cpu.o \
	 rom.o cartridge.o 6502core.o debug.o ram.o \
	 ppu.o dma.o ioapu.o joypad.o
	 
#	 kbhit.o


#LIBS=-L../libs -pthread -lrt -lncurses
LIBS=-L../libs -lmotones -pthread -lrt \
		  -L./ppucore -lppucore \
		  -L./joypad -ljoypad

INCLUDE=-I../include -Ippucore

CFLAGS=-g $(INCLUDE) -Wall
LFLAGS=$(LIBS) `pkg-config gtk+-2.0 gthread-2.0 --libs`

all:	$(BIN)

$(BIN): $(OBJS) ../libs ppucore/libppucore.a joypad/libjoypad.a
	gcc -o $(BIN) $(OBJS) $(LFLAGS)


ppucore/libppucore.a: ppucore/*.c
	make -C ppucore

joypad/libjoypad.a: joypad/*.c
	make -C joypad

.c.o: 
	gcc $(CFLAGS) -c $<

6502core.o: 6502core.c
	./opcode-gen.sh
	gcc $(CFLAGS) -c $<

test:
	make
	./test.sh

clean:
	make -C ppucore clean
	make -C joypad clean
	-rm $(OBJS) $(BIN) opcode \
		
tag:
	cd .. && make tag


