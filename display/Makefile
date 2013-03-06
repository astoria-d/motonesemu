
BIN=vgadisp

DRIVER=dummy-driver

OBJS=display.o window.o communication.o
	 


LIBS=-L../libs -lmotones -pthread

INCLUDE=-I../include -I/usr/include/gtk-2.0

CFLAGS=-g $(INCLUDE) `pkg-config gtk+-2.0 --cflags` -Wall

LFLAGS=-g $(LIBS) `pkg-config gtk+-2.0 gthread-2.0 --libs`

all:	$(BIN) $(DRIVER)

$(BIN): $(OBJS) ../libs
	gcc -o $(BIN) $(OBJS) $(LFLAGS)

$(DRIVER): $(DRIVER).o
	gcc -o $(DRIVER) $(CFLAGS) $(DRIVER).c

.c.o: 
	gcc $(CFLAGS) -c $<

clean:
	-rm $(OBJS) $(BIN) $(DRIVER) $(DRIVER).o
		
tag:
	cd .. && make tag


