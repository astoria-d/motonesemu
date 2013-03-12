
#BIN=ppucore
BIN=dummy-driver2

DUMMY_OBJS=dummy-driver2.o debug.o cartridge.o 

OBJS=ppucore.o vram.o vscreen.o palette.o \
	 vga_xfer.o $(DUMMY_OBJS)
	 
LIBS=-L../../libs -lmotones -pthread

INCLUDE=-I../../include

CFLAGS=-g $(INCLUDE) -Wall

LFLAGS=-g $(LIBS) -Wall

all:	$(BIN) $(DRIVER)

$(BIN): $(OBJS) ../../libs
	gcc -o $(BIN) $(OBJS) $(LFLAGS)

.c.o: 
	gcc $(CFLAGS) -c $<

clean:
	-rm $(OBJS) $(BIN)
		
tag:
	cd ../.. && make tag


