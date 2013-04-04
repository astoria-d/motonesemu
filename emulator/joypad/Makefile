
BIN=libjoypad.a
TEST_BIN=jp-driver

DUMMY_OBJS=jp-driver.o

OBJS=joypad.o
	 
LIBS=-L../../libs -L./ -lmotones -lppucore

INCLUDE=-I../../include -I../

CFLAGS=-g $(INCLUDE) -Wall

LFLAGS=-g $(LIBS) -Wall

all:	$(BIN)
test:	$(TEST_BIN)

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


