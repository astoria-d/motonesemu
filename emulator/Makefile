
BIN=motonesemu

OBJS=emu-main.o clock.o tools.o bus.o cpu.o \


LIBS=-L../libs -pthread

INCLUDE=-I../include

CFLAGS=-g $(INCLUDE)

all:	$(BIN)

$(BIN): $(OBJS) ../libs
	gcc -o $(BIN) $(OBJS) $(LIBS)

.c.o: 
	gcc $(CFLAGS) -c $<

test:
	make
	./test.sh

clean:
	-rm $(OBJS) $(BIN) \
		



