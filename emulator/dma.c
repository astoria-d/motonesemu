
#include "tools.h"

static unsigned char sprite_dma_reg;

void set_dma_data(unsigned char data) {
    dprint("set_dma_data: %02x\n", data);
    sprite_dma_reg = data;
}

int init_dma(void) {

    sprite_dma_reg = 0;
    return TRUE;
}

void clean_dma(void) {

}

