#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "tools.h"
#include "clock.h"
#include "mapper.h"

void release_bus(void);

struct rom_pin {
    unsigned int rw     :1;     /*assert on write.*/
    unsigned int ce     :1;     /*chip enable*/
};

static struct rom_pin rom_pin_status;
static unsigned short rom_addr;
static unsigned char rom_data;

#define ROM_32K 0x8000

static unsigned char * rom_buffer;

/*mapper debugger stab.*/
mp_dbg_get_byte_t mp_dbg_get_byte;
mp_dbg_get_short_t mp_dbg_get_short;
mp_dbg_get_byte_t dbg_rom_get_byte_in;
mp_dbg_get_short_t dbg_rom_get_short_in;

int load_prg_rom(FILE* cartridge, int num_rom_bank) {
    int len;

    rom_buffer = malloc(ROM_32K);
    if (rom_buffer == NULL)
        return FALSE;
    len = fread(rom_buffer, 1, ROM_32K, cartridge);
    if (len != ROM_32K)
        return FALSE;
    return TRUE;
}

void set_rom_addr(unsigned short addr) {
    rom_addr = addr;
}

unsigned char get_rom_data(void) {
    return rom_data;
}

void set_rom_ce_pin(int ce) {
    rom_pin_status.ce = ce;
    //let rom write the value on the bus.
    if (ce) {
            rom_data = rom_buffer[rom_addr];
            release_bus();
    }
}

int init_rom(void) {
    rom_buffer = NULL;
    rom_addr = 0;
    rom_data = 0;
    rom_pin_status.rw = 0;
    rom_pin_status.ce = 0;
    unsigned char dbg_rom_get_byte(unsigned short offset);
    unsigned short dbg_rom_get_short(unsigned short offset);

    /*setup mapper debug stab.*/
    dbg_rom_get_byte_in = (mp_dbg_get_byte != NULL ? mp_dbg_get_byte : dbg_rom_get_byte);
    dbg_rom_get_short_in = (mp_dbg_get_short != NULL ? mp_dbg_get_short : dbg_rom_get_short);
    return TRUE;
}

void clean_rom(void) {
    if (rom_buffer)
        free(rom_buffer);
}

/*
 * for debug.c
 * */
unsigned char dbg_rom_get_byte(unsigned short offset) {
    return rom_buffer[offset];
}
unsigned short dbg_rom_get_short(unsigned short offset) {
    unsigned short ret;
    //little endian. low byte first.
    ret = rom_buffer[offset];
    ret |= (rom_buffer[offset + 1] << 8);
    return ret;
}

