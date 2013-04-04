#include <stdlib.h>
#include <string.h>

#include "tools.h"
#include "vga.h"
#include "vram.h"
#include "ppucore.h"
#include "sprite.h"

void palette_index_to_rgb15(unsigned char index, struct rgb15* rgb);


static unsigned char * sprite_ram;

void spr_ram_tbl_set(unsigned short offset, unsigned char data) {
    sprite_ram[offset & SPR_RAM_ADDR_MASK] = data;
}

unsigned char spr_ram_tbl_get(unsigned short offset) {
    return sprite_ram[offset & SPR_RAM_ADDR_MASK];
}

int init_sprite(void) {
    sprite_ram = malloc(SPRITE_RAM_SIZE);
    if (sprite_ram == NULL)
        return FALSE;
    memset(sprite_ram, 0, SPRITE_RAM_SIZE);
    return TRUE;
}

void clean_sprite(void) {
    free(sprite_ram);
}

