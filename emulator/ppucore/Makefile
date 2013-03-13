
BIN=libppucore.a
TEST_BIN=dummy-driver2

DUMMY_OBJS=dummy-driver2.o debug.o cartridge.o 

OBJS=ppucore.o vram.o vscreen.o palette.o \
	 vga_xfer.o
	 
LIBS=-L../../libs -L./ -lmotones -pthread -lrt -lppucore

INCLUDE=-I../../include

CFLAGS=-g $(INCLUDE) -Wall

LFLAGS=-g $(LIBS) -Wall

all:	$(BIN) $(TEST_BIN)

$(TEST_BIN): $(BIN) $(DUMMY_OBJS) ../../libs
	gcc -o $(TEST_BIN) $(DUMMY_OBJS) $(LFLAGS)

$(BIN): $(OBJS) ../../libs
	ar rcs $(BIN) $(OBJS)

.c.o: 
	gcc $(CFLAGS) -c $<

clean:
	-rm $(OBJS) $(DUMMY_OBJS) $(BIN) $(TEST_BIN)
		
tag:
	cd ../.. && make tag


