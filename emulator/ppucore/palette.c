#include <string.h>
#include "tools.h"
#include "vram.h"

#define PALETTE_SIZE    64
static struct rgb15 NES_PALETTE_RGB15[PALETTE_SIZE];

//copied from 
//http://hlc6502.web.fc2.com/NesPal2.htm
//
static const unsigned int NES_PALETTE_RGB24[PALETTE_SIZE] = {
    0x787878, 0x2000B0, 0x2800B8, 0x6010A0, 0x982078, 0xB01030, 0xA03000, 0x784000, 
    0x485800, 0x386800, 0x386C00, 0x306040, 0x305080, 0x000000, 0x000000, 0x000000,
    0xB0B0B0, 0x4060F8, 0x4040FF, 0x9040F0, 0xD840C0, 0xD84060, 0xE05000, 0xC07000, 
    0x888800, 0x50A000, 0x48A810, 0x48A068, 0x4090C0, 0x000000, 0x000000, 0x000000,

    0xFFFFFF, 0x60A0FF, 0x5080FF, 0xA070FF, 0xF060FF, 0xFF60B0, 0xFF7830, 0xFFA000, 
    0xE8D020, 0x98E800, 0x70F040, 0x70E090, 0x60D0E0, 0x787878, 0x000000, 0x000000,
    0xFFFFFF, 0x90D0FF, 0xA0B8FF, 0xC0B0FF, 0xE0B0FF, 0xFFB8E8, 0xFFC8B8, 0xFFD8A0, 
    0xFFF090, 0xC8F080, 0xA0F0A0, 0xA0FFC8, 0xA0FFF0, 0xA0A0A0, 0x000000, 0x000000
};

static void from_rgb24(unsigned int rgb24, struct rgb15* rgb) {
    //dprint("from_rgb24: %02x >> %02x\n", rgb24 >> 16, colto5bit(rgb24 >> 16));
    rgb->r = colto5bit((rgb24 >> 16));
    rgb->g = colto5bit((rgb24 >> 8) & 0xFF);
    rgb->b = colto5bit(rgb24 & 0xFF);
}

void set_monocolor (int mono) {
    if (mono) {
        int i;
        for (i = 0; i < PALETTE_SIZE; i++) {
            NES_PALETTE_RGB15[i].r = 0x1f;
            NES_PALETTE_RGB15[i].g = 0x1f;
            NES_PALETTE_RGB15[i].b = 0x1f;
        }
    }
    else {
        int i;
        for (i = 0; i < PALETTE_SIZE; i++) {
            from_rgb24(NES_PALETTE_RGB24[i], &NES_PALETTE_RGB15[i]);
        }
    }
}


/*
 * bank: 0 >> bg palette
 * bank: 1 >> sprite palette
 * */
void palette_index_to_rgb15(unsigned char index, struct rgb15* rgb) {
    index = index & (PALETTE_SIZE - 1);
    rgb->r = NES_PALETTE_RGB15[index].r;
    rgb->g = NES_PALETTE_RGB15[index].g;
    rgb->b = NES_PALETTE_RGB15[index].b;
}

int palette_init(void) {
    int i;
    for (i = 0; i < PALETTE_SIZE; i++) {
        from_rgb24(NES_PALETTE_RGB24[i], &NES_PALETTE_RGB15[i]);
    }

    return TRUE;
}

