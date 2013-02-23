
#include <string.h>
#include "tools.h"
#include "bus.h"

struct cpu_pin {
    unsigned int rw     :1;     /*assert on write.*/
    unsigned int nmi    :1;     /*input*/
    unsigned int irq    :1;     /*input*/
    unsigned int reset  :1;     /*input*/
    unsigned int ready  :1;
    unsigned int clock  :1;     /*not used*/
};

static unsigned short addr_bus;
static unsigned char data_bus;
static struct cpu_pin pin_status;

/*
 * NES memory map
 * 0x0000   -   0x07FF      RAM
 * 0x2000   -   0x2007      I/O PPU
 * 0x4000   -   0x401F      I/O APU
 * 0x6000   -   0x7FFF      battery backup ram
 * 0x8000   -   0xFFFF      PRG-ROM
 * */

#define IO_PPU_BIT  0x2000
#define IO_APU_BIT  0x6000
#define ROM_BIT     0x8000

#define RAM_MASK    0x07FF
#define IO_PPU_MASK 0x0007
#define IO_APU_MASK 0x001F
#define ROM_MASK    0x7FFF

void set_bus_addr(unsigned short addr) {
    if (addr & ROM_BIT) {
        set_rom_addr(addr & ROM_MASK);
        set_rom_ce_pin(TRUE);
    }
    else if (addr & IO_APU_BIT) {
    }
    else if (addr & IO_PPU_BIT) {
    }
    else {
        /*case ram*/
    }
    addr_bus = addr;
}

void set_bus_data(unsigned char data){
#warning write memory not worked yet!!
    data_bus = data;
}

char get_bus_data(void){
    if (addr_bus & ROM_BIT) {
        data_bus = get_rom_data();
        set_rom_ce_pin(FALSE);
    }
    else if (addr_bus & IO_APU_BIT) {
    }
    else if (addr_bus & IO_PPU_BIT) {
    }
    else {
        /*case ram*/
    }
    return data_bus;
}

/*
 * rw = 1 on write.
 * */
void set_rw_pin(int rw) {
    pin_status.rw = rw;
}

int init_bus(void) {
    addr_bus = 0;
    data_bus = 0;
    memset(&pin_status, 0, sizeof(struct cpu_pin));
    return TRUE;
}

void clean_bus(void){
}

