
BIN=vgadisp

OBJS=display.o
	 


LIBS=-L../libs -pthread

INCLUDE=-I../include

CFLAGS=-g $(INCLUDE)

all:	$(BIN)

$(BIN): $(OBJS) ../libs
	gcc -o $(BIN) $(OBJS) $(LIBS)

.c.o: 
	gcc $(CFLAGS) -c $<

clean:
	-rm $(OBJS) $(BIN)
		



