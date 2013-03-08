
#BIN=ppucore
BIN=dummy-driver2

OBJS=dummy-driver2.o ppucore.o
	 
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
	cd .. && make tag


