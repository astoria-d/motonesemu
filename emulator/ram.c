#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "tools.h"
#include "clock.h"

void release_bus(void);

struct ram_pin {
    unsigned int ce     :1;     /*chip enable*/
    unsigned int oe     :1;     /*assert on read ready.*/
    unsigned int we     :1;     /*assert on write.*/
};

static struct ram_pin ram_pin_status;
static unsigned short ram_addr;
static unsigned char ram_data;

#define RAM_2K 0x0800

unsigned char * ram_buffer;

void set_ram_addr(unsigned short addr) {
    ram_addr = addr;
}

unsigned char get_ram_data(void) {
    return ram_data;
}

void set_ram_data(unsigned char data) {
    ram_data = data;
}

void set_ram_oe_pin(int oe) {
    ram_pin_status.oe = oe;
}

void set_ram_we_pin(int we) {
    ram_pin_status.we = we;
}

void set_ram_ce_pin(int ce) {
    ram_pin_status.ce = ce;
    //let ram i/o on the bus.
    if (ce) {
        if (ram_pin_status.oe) {
            //read cycle
            ram_data = ram_buffer[ram_addr];
        }
        else if (ram_pin_status.we) {
            //write cycle
            ram_buffer[ram_addr] = ram_data;
        }
        release_bus();
    }
}

int init_ram(void) {
    ram_buffer = malloc(RAM_2K);
    if (!ram_buffer)
        return FALSE;

    ram_addr = 0;
    ram_data = 0;
    ram_pin_status.oe = 0;
    ram_pin_status.we = 0;
    ram_pin_status.ce = 0;

    return TRUE;
}

void clean_ram(void) {
    if (ram_buffer)
        free(ram_buffer);
}

/*
 * for debug.c
 * */
unsigned char dbg_ram_get_byte(unsigned short offset) {
    return ram_buffer[offset];
}
unsigned short dbg_ram_get_short(unsigned short offset) {
    unsigned short ret;
    ret = ram_buffer[offset];
    ret |= (ram_buffer[offset + 1] << 8);
    return ret;
}

