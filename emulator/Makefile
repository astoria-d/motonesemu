
BIN=motonesemu

OBJS=emu-main.o clock.o bus.o cpu.o \
	 rom.o cartridge.o 6502core.o debug.o ram.o \
	 ppu.o dma.o ioapu.o
	 
#	 kbhit.o


#LIBS=-L../libs -pthread -lrt -lncurses
LIBS=-L../libs -L./ppucore -lmotones -lppucore -pthread -lrt

INCLUDE=-I../include -Ippucore

CFLAGS=-g $(INCLUDE) -Wall

all:	$(BIN)

$(BIN): $(OBJS) ../libs ppucore/libppucore.a
	gcc -o $(BIN) $(OBJS) $(LIBS)

ppucore/libppucore.a: ppucore/*.c
	make -C ppucore

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
	-rm $(OBJS) $(BIN) opcode \
		
tag:
	cd .. && make tag


