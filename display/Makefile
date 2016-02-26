
BIN=vgadisp

DRIVER=dummy-driver

DRIVER_OBJS=dummy-driver.o


LIBS=-L../libs -lmotones -pthread

ifeq ($(linux_env),1)
    INCLUDE=-I../include -I/usr/include/gtk-2.0
    CFLAGS=-g $(INCLUDE) `pkg-config gtk+-2.0 --cflags` -Wall
    LFLAGS=-g $(LIBS) `pkg-config gtk+-2.0 gthread-2.0 --libs`
    OBJS=display.o window.o
else
    INCLUDE=-I../include
    CFLAGS=-g $(INCLUDE) -Wall
    LFLAGS=-g $(LIBS)
    OBJS=display.o no-window.o
endif


all:	$(BIN) $(DRIVER)

$(BIN): $(OBJS) ../libs
	gcc -o $(BIN) $(OBJS) $(LFLAGS)

$(DRIVER): $(DRIVER_OBJS)
	gcc -o $(DRIVER) $(DRIVER_OBJS) $(LIBS)

.c.o: 
	gcc $(CFLAGS) -c $<

clean:
	-rm $(OBJS) $(BIN) $(DRIVER) $(DRIVER).o
		
tag:
	cd .. && make tag


