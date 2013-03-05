
BIN=motonesemu

OBJS=emu-main.o clock.o tools.o bus.o cpu.o \
	 rom.o cartridge.o 6502core.o debug.o ram.o \
	 ppu.o
	 
#	 kbhit.o


#LIBS=-L../libs -pthread -lrt -lncurses
LIBS=-L../libs -pthread -lrt

INCLUDE=-I../include

CFLAGS=-g $(INCLUDE)

all:	$(BIN)

$(BIN): $(OBJS) ../libs
	gcc -o $(BIN) $(OBJS) $(LIBS)

.c.o: 
	gcc $(CFLAGS) -c $<

6502core.o: 6502core.c
	./opcode-gen.sh
	gcc $(CFLAGS) -c $<

test:
	make
	./test.sh

clean:
	-rm $(OBJS) $(BIN) opcode \
		



