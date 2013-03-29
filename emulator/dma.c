
#include "tools.h"
#include "sprite.h"
#include "clock.h"
#include "bus.h"

#define DMA_ADDR_MULTIPLE       0x100

void spr_ram_tbl_set(unsigned short offset, unsigned char data);
int register_cpu_clock(void);
int unregister_cpu_clock(void);

static unsigned short dma_ram_addr;
static unsigned char dma_ram_data;
static unsigned char dma_spr_addr;
static int dma_cnt;

static void get_ram_data(void) {
    if (dma_cnt > 0)
        release_bus();
    set_rw_pin(0);
    set_bus_addr(dma_ram_addr++);
    start_bus();
    dma_ram_data = get_bus_data();
    end_bus();

    /*
    dprint("dma %d, ram:%02x @%04x\n", 
            dma_cnt, dma_ram_data, dma_ram_addr);
    */

    //keep the bus ownership.
    take_bus();
}

/*
 * clock handler.
 * */
static int clock_dma(void) {
    if (dma_cnt % 2 == 0) {
        //load ram value
        get_ram_data();

    }
    else {
        //store into sprite ram
        spr_ram_tbl_set(dma_spr_addr++, dma_ram_data);
    }

    if (dma_cnt == SPRITE_RAM_SIZE * 2 - 1) {
        //dma done.
        release_bus();
        //restore clock handler.
        unregister_clock_hander(clock_dma);
        register_cpu_clock();
    }
    dma_cnt++;

    return TRUE;
}

void set_dma_data(unsigned char data) {
    //dprint("set_dma_data: %02x\n", data);
    dma_ram_addr = data * DMA_ADDR_MULTIPLE;
    dma_spr_addr = 0;
    //start dma work
    register_clock_hander(clock_dma, CPU_DEVIDER);
    //stop cpu clock
    unregister_cpu_clock();
    dma_cnt = 0;
}

int init_dma(void) {
    dma_ram_addr = 0;
    dma_spr_addr = 0;
    dma_ram_data = 0;
    dma_cnt = 0;

    return TRUE;
}

void clean_dma(void) {

}

