
BIN=vgadisp

DRIVER=dummy-driver

OBJS=display.o
	 


LIBS=-L../libs -lmotones -pthread

INCLUDE=-I../include

CFLAGS=-g $(INCLUDE)

all:	$(BIN) $(DRIVER)

$(BIN): $(OBJS) ../libs
	gcc -o $(BIN) $(OBJS) $(LIBS)

$(DRIVER): $(DRIVER).o
	gcc -o $(DRIVER) $(CFLAGS) $(DRIVER).c

.c.o: 
	gcc $(CFLAGS) -c $<

clean:
	-rm $(OBJS) $(BIN) $(DRIVER) $(DRIVER).o
		
tag:
	cd .. && make tag


