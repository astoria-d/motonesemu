
BIN=vgadisp

DRIVER=dummy-driver

OBJS=display.o
	 


LIBS=-L../libs -pthread

INCLUDE=-I../include

CFLAGS=-g $(INCLUDE)

all:	$(BIN) $(DRIVER)

$(BIN): $(OBJS) ../libs
	gcc -o $(BIN) $(OBJS) $(LIBS)

$(DRIVER): $(DRIVER).o
	gcc -o $(DRIVER) $(DRIVER).c

.c.o: 
	gcc $(CFLAGS) -c $<

clean:
	-rm $(OBJS) $(BIN) $(DRIVER) $(DRIVER).o
		
tag:
	cd .. && make tag


