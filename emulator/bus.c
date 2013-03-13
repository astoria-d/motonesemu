#include <string.h>
#include <semaphore.h>

#include "tools.h"
#include "bus.h"
#include "rom.h"
#include "ram.h"

unsigned char dbg_rom_get_byte(unsigned short offset);
unsigned short dbg_rom_get_short(unsigned short offset);
unsigned char dbg_ram_get_byte(unsigned short offset);
unsigned short dbg_ram_get_short(unsigned short offset);

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
static sem_t sem_bus_wait;

/*
 * NES memory map
 * 0x0000   -   0x07FF      RAM
 * 0x0800   -   0x1FFF      mirror RAM
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

unsigned char dbg_get_byte(unsigned short addr) {
    if (addr & ROM_BIT) {
        return dbg_rom_get_byte(addr & ROM_MASK);
    }
    else if (addr & IO_APU_BIT) {
        return 0;
    }
    else if (addr & IO_PPU_BIT) {
        return 0;
    }
    else {
        return dbg_ram_get_byte(addr & ROM_MASK);
    }
}
unsigned short dbg_get_short(unsigned short addr) {
    if (addr & ROM_BIT) {
        return dbg_rom_get_short(addr & ROM_MASK);
    }
    else if (addr & IO_APU_BIT) {
        return 0;
    }
    else if (addr & IO_PPU_BIT) {
        return 0;
    }
    else {
        return dbg_ram_get_short(addr & ROM_MASK);
    }
}


void release_bus(void) {
    pin_status.ready = 1;
    sem_post(&sem_bus_wait);
}

/*
 * this function blocks when accessing rom/ram device.
 * */
void start_bus(void) {
    if (addr_bus & ROM_BIT) {
        /*case rom*/
        pin_status.ready = 0;
        set_rom_ce_pin(TRUE);

        //wait for the bus ready.
        sem_wait(&sem_bus_wait);
    }
    else if (addr_bus & IO_APU_BIT) {
    }
    else if (addr_bus & IO_PPU_BIT) {
    }
    else {
        /*case ram*/
        pin_status.ready = 0;
        set_ram_ce_pin(TRUE);

        //wait for the bus ready.
        sem_wait(&sem_bus_wait);
    }
}

void end_bus(void) {
    if (addr_bus & ROM_BIT) {
        /*case rom*/
        set_rom_ce_pin(FALSE);
    }
    else if (addr_bus & IO_APU_BIT) {
    }
    else if (addr_bus & IO_PPU_BIT) {
    }
    else {
        /*case ram*/
        set_ram_ce_pin(FALSE);
    }
}

void set_bus_addr(unsigned short addr) {
    if (!pin_status.ready)
        return;

    if (addr & ROM_BIT) {
        /*case rom*/
        set_rom_addr(addr & ROM_MASK);
    }
    else if (addr & IO_APU_BIT) {
    }
    else if (addr & IO_PPU_BIT) {
    }
    else {
        /*case ram*/
        set_ram_addr(addr & RAM_MASK);
        if (pin_status.rw) {
            set_ram_oe_pin(FALSE);
            set_ram_we_pin(TRUE);
        }
        else {
            set_ram_oe_pin(TRUE);
            set_ram_we_pin(FALSE);
        }
    }
    addr_bus = addr;
}

void set_bus_data(unsigned char data){
    if (!pin_status.ready)
        return;

    if (addr_bus & ROM_BIT) {
    }
    else if (addr_bus & IO_APU_BIT) {
    }
    else if (addr_bus & IO_PPU_BIT) {
    }
    else {
        /*case ram*/
        set_ram_data(data);
    }
    data_bus = data;
}

char get_bus_data(void) {
    if (!pin_status.ready)
        return 0;

    if (addr_bus & ROM_BIT) {
        data_bus = get_rom_data();
    }
    else if (addr_bus & IO_APU_BIT) {
    }
    else if (addr_bus & IO_PPU_BIT) {
    }
    else {
        /*case ram*/
        data_bus = get_ram_data();
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
    int ret;

    addr_bus = 0;
    data_bus = 0;
    memset(&pin_status, 0, sizeof(struct cpu_pin));
    pin_status.ready = 1;

    ret = sem_init(&sem_bus_wait, 0, 0);
    if (ret != RT_OK)
        return FALSE;

    return TRUE;
}

void clean_bus(void){
    sem_destroy(&sem_bus_wait);
}

