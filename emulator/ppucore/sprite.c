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

void spr_ram_data_get(unsigned char index, unsigned char *x, unsigned char *y, 
        unsigned char *tile_id, struct sprite_attr *sa) {
    //index is multiple of 4.
    index *= 4;
    index &= SPR_RAM_ADDR_MASK;

    *y = sprite_ram[index++];
    *tile_id = sprite_ram[index++];
    memcpy(sa, sprite_ram + index++, sizeof(struct sprite_attr));
    *x = sprite_ram[index++];
}

int sprite_init(void) {
    sprite_ram = malloc(SPRITE_RAM_SIZE);
    if (sprite_ram == NULL)
        return FALSE;
    memset(sprite_ram, 0, SPRITE_RAM_SIZE);
    return TRUE;
}

void clean_sprite(void) {
    free(sprite_ram);
}

