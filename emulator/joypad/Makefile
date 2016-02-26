
BIN=libjoypad.a
TEST_BIN=jp-driver

DUMMY_OBJS=jp-driver.o
	 
LIBS=-L../../libs -L./ -ljoypad -lmotones -pthread
#LIBS=-L./ -lmotones -ljoypad

INCLUDE=-I../../include -I../

ifeq ($(linux_env),1)
    CFLAGS=-g $(INCLUDE) -Wall `pkg-config gtk+-2.0 --cflags`
    LFLAGS=-g $(LIBS) `pkg-config gtk+-2.0 gthread-2.0 --libs`
    OBJS=joypad-wnd.o
else
    CFLAGS=-g $(INCLUDE) -Wall
    LFLAGS=-g $(LIBS)
    OBJS=joypad-no-wnd.o
endif

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


