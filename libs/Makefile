
BIN=libmotones.a

OBJS=tools.o

INCLUDE=-I../include

CFLAGS=-g $(INCLUDE)

all:	$(BIN)

$(BIN): $(OBJS)
	ar rcs $(BIN) $(OBJS)

.c.o: 
	gcc $(CFLAGS) -c $<

clean:
	-rm $(OBJS) $(BIN)

