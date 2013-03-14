#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tools.h"
#include "ppucore.h"
#include "vram.h"
#include "sprite.h"

void palette_index_to_rgb15(unsigned char index, struct rgb15* rgb);
void dump_mem(const char* msg, unsigned short base, 
        unsigned short offset, unsigned char* buf, int size);
void set_bgtile(int tile_id);

/*
 * NES vram memory map
 *
 * 0x0000   -   0x0FFF      pattern table 0
 * 0x1000   -   0x1FFF      pattern table 1
 *
 * 0x2000   -   0x23BF      name table 0
 * 0x23C0   -   0x23FF      attribute table 0
 * 0x2400   -   0x27BF      name table 1
 * 0x27C0   -   0x27FF      attribute table 1
 * 0x2800   -   0x2BBF      name table 2
 * 0x2BC0   -   0x2BFF      attribute table 2
 * 0x2C00   -   0x2FBF      name table 3
 * 0x2FC0   -   0x2FFF      attribute table 3
 * 0x3000   -   0x3EFF      mirror name/attr tbl
 *
 * 0x3F00   -   0x3F0F      bg palette tble
 * 0x3F10   -   0x3F1F      sprite palette tble
 * 0x3F20   -   0x3FFF      mirror palette tble
 *
 * 0x4000   -   0xFFFF      mirror 0x0000-0x3FFFF
 * */

#define PATTERN_TBL_SIZE    0x1000
#define NAME_TBL_SIZE       V_SCREEN_TILE_SIZE * H_SCREEN_TILE_SIZE
#define ATTR_TBL_SIZE       (VIRT_SCREEN_TILE_SIZE * VIRT_SCREEN_TILE_SIZE \
                            / ATTR_GROUP_UNIT / ATTR_UNIT_PER_BYTE)
#define PALETTE_TBL_SIZE    16
#define SPRITE_RAM_SIZE     0xff

#define PATTERN_ADDR_MASK       (PATTERN_TBL_SIZE - 1)
#define ATTR_TBL_ADDR_MASK      (ATTR_TBL_SIZE - 1)
#define PALETTE_TBL_ADDR_MASK   (PALETTE_TBL_SIZE - 1)
#define SPR_RAM_ADDR_MASK       (SPRITE_RAM_SIZE - 1)

#define PPU_ADDR_MASK       (0x4000 - 1)
#define PALETTE_START       0x3F00
#define NAME_ATTR_MASK1      (0x3000 - 1)
#define NAME_ATTR_MASK2      (0x0400 - 1)
#define PALETTE_SPRITE_BIT  0x10

#define NAME0_START         PATTERN_TBL_SIZE * 2
#define ATTR0_START         NAME0_START + NAME_TBL_SIZE
#define NAME1_START         ATTR0_START + ATTR_TBL_SIZE
#define ATTR1_START         NAME1_START + NAME_TBL_SIZE
#define NAME2_START         ATTR1_START + ATTR_TBL_SIZE
#define ATTR2_START         NAME2_START + NAME_TBL_SIZE
#define NAME3_START         ATTR2_START + ATTR_TBL_SIZE
#define ATTR3_START         NAME3_START + NAME_TBL_SIZE

/*vram definition*/
static unsigned char * sprite_ram;

static unsigned char * bg_palette_tbl;
static unsigned char * spr_palette_tbl;

static unsigned char * name_tbl0;
static unsigned char * name_tbl1;
static unsigned char * name_tbl2;
static unsigned char * name_tbl3;

static unsigned char * attr_tbl0;
static unsigned char * attr_tbl1;
static unsigned char * attr_tbl2;
static unsigned char * attr_tbl3;

static unsigned char * pattern_tbl0;
static unsigned char * pattern_tbl1;

/*
 * VRAM get/set functions....
 *
 * */

static unsigned char pattern_tbl_get(unsigned char bank, unsigned short offset) {
    if (bank == 0)
        return pattern_tbl0[offset];
    else
        return pattern_tbl1[offset];
}

static unsigned char name_tbl_get(unsigned char bank, unsigned short offset) {
    if (bank == 0)
        return name_tbl0[offset];
    else if (bank == 1)
        return name_tbl1[offset];
    else if (bank == 2)
        return name_tbl2[offset];
    else
        return name_tbl3[offset];
}

static void name_tbl_set(unsigned char bank, unsigned short offset, unsigned char data) {
    if (bank == 0)
        name_tbl0[offset] = data;
    else if (bank == 1)
        name_tbl1[offset] = data;
    else if (bank == 2)
        name_tbl2[offset] = data;
    else
        name_tbl3[offset] = data;
}


static unsigned char attr_tbl_get(unsigned char bank, unsigned short offset) {
    if (bank == 0)
        return attr_tbl0[offset];
    else if (bank == 1)
        return attr_tbl1[offset];
    else if (bank == 2)
        return attr_tbl2[offset];
    else
        return attr_tbl3[offset];
}

static void attr_tbl_set(unsigned char bank, unsigned short offset, unsigned char data) {
    if (bank == 0)
        attr_tbl0[offset] = data;
    else if (bank == 1)
        attr_tbl1[offset] = data;
    else if (bank == 2)
        attr_tbl2[offset] = data;
    else
        attr_tbl3[offset] = data;
}


static unsigned char spr_palette_tbl_get(unsigned short offset) {
    return spr_palette_tbl[offset];
}

static void spr_palette_tbl_set(unsigned short offset, unsigned char data) {
    spr_palette_tbl[offset] = data;
}

static unsigned char bg_palette_tbl_get(unsigned short offset) {
    return bg_palette_tbl[offset];
}

static void bg_palette_tbl_set(unsigned short offset, unsigned char data) {
    bg_palette_tbl[offset] = data;
}


static unsigned char spr_ram_tbl_get(unsigned short offset) {
    return sprite_ram[offset];
}

static void spr_ram_tbl_set(unsigned short offset, unsigned char data) {
    sprite_ram[offset] = data;
}

void vram_data_set(unsigned short addr, unsigned char data) {

    addr &= PPU_ADDR_MASK;

    if (addr < 2 * PATTERN_TBL_SIZE) {
        //do nothing. pattern table is read only.
    }
    else if (addr >= PALETTE_START) {
        if (addr & PALETTE_SPRITE_BIT)
            bg_palette_tbl_set(addr & PALETTE_TBL_ADDR_MASK, data);
        else 
            bg_palette_tbl_set(addr & PALETTE_TBL_ADDR_MASK, data);
    }
    else {
        if (addr < NAME0_START) {
            name_tbl_set(0, addr - NAME0_START, data);
        }
        else if (addr < ATTR0_START) {
            attr_tbl_set(0, addr - ATTR0_START, data);
        }
        else if (addr < NAME1_START) {
            name_tbl_set(1, addr - NAME1_START, data);
        }
        else if (addr < ATTR1_START) {
            attr_tbl_set(1, addr - ATTR1_START, data);
        }
        else if (addr < NAME2_START) {
            name_tbl_set(2, addr - NAME2_START, data);
        }
        else if (addr < ATTR2_START) {
            attr_tbl_set(2, addr - ATTR2_START, data);
        }
        else if (addr < NAME3_START) {
            name_tbl_set(3, addr - NAME3_START, data);
        }
        else {
            attr_tbl_set(3, addr - ATTR3_START, data);
        }
    }
}

unsigned char vram_data_get(unsigned short addr) {

    addr &= PPU_ADDR_MASK;

    if (addr < PATTERN_TBL_SIZE) {
        return name_tbl_get(0, addr & PATTERN_ADDR_MASK);
    }
    if (addr < 2 * PATTERN_TBL_SIZE) {
        return name_tbl_get(1, addr & PATTERN_ADDR_MASK);
    }
    else if (addr >= PALETTE_START) {
        if (addr & PALETTE_SPRITE_BIT)
            return bg_palette_tbl_get(addr & PALETTE_TBL_ADDR_MASK);
        else 
            return bg_palette_tbl_get(addr & PALETTE_TBL_ADDR_MASK);
    }
    else {
        if (addr < NAME0_START) {
            return name_tbl_get(0, addr - NAME0_START);
        }
        else if (addr < ATTR0_START) {
            return attr_tbl_get(0, addr - ATTR0_START);
        }
        else if (addr < NAME1_START) {
            return name_tbl_get(1, addr - NAME1_START);
        }
        else if (addr < ATTR1_START) {
            return attr_tbl_get(1, addr - ATTR1_START);
        }
        else if (addr < NAME2_START) {
            return name_tbl_get(2, addr - NAME2_START);
        }
        else if (addr < ATTR2_START) {
            return attr_tbl_get(2, addr - ATTR2_START);
        }
        else if (addr < NAME3_START) {
            return name_tbl_get(3, addr - NAME3_START);
        }
        else {
            return attr_tbl_get(3, addr - ATTR3_START);
        }
    }
    return 0;
}

/* VRAM manipulation... */

int show_background(void) {
    int i;
    for (i = 0; i < H_SCREEN_TILE_SIZE * V_SCREEN_TILE_SIZE; i++) {
#warning update bg must be more efficient. update only changed tile.
        set_bgtile(i);
    }
    return TRUE;
}

static int attr_index_to_gp(int tile_index) {
    int tile_x, tile_y, gp_x, gp_y;

    tile_x = tile_index % H_SCREEN_TILE_SIZE;
    tile_y = tile_index / H_SCREEN_TILE_SIZE;

    gp_x = tile_x / ATTR_GROUP_UNIT;
    gp_y = tile_y / ATTR_GROUP_UNIT;
    //dprint("tile_x:%d, y:%d, gp_x:%d, y:%d\n", tile_x, tile_y, gp_x, gp_y);

    return gp_x + gp_y * 8;
}

void load_attribute(unsigned char bank, int tile_index, struct palette *plt) {
    int gp_index;
    unsigned char data;
    struct palette_unit pu;
    int palette_group;
    unsigned short palette_addr;
    unsigned char pi;
    int tile_x, tile_y;
    int in_x, in_y;

    gp_index = attr_index_to_gp(tile_index);
    data = attr_tbl_get(bank, gp_index);
    memcpy(&pu, &data, sizeof(pu));

    tile_x = tile_index % H_SCREEN_TILE_SIZE;
    tile_y = tile_index / H_SCREEN_TILE_SIZE;
    in_x = tile_x % (ATTR_GROUP_UNIT);
    in_y = tile_y % (ATTR_GROUP_UNIT);
    if (in_y < 2) {
        if (in_x < 2) {
            palette_group = pu.bit01;
        }
        else {
            palette_group = pu.bit23;
        }
    }
    else {
        if (in_x < 2) {
            palette_group = pu.bit45;
        }
        else {
            palette_group = pu.bit67;
        }
    }
    /*
    dprint("tile_index: %d, gp_index: %d\n", tile_index, gp_index);
    dprint("in_x: %d, in_y: %d\n", in_x, in_y);
    dprint("pu bit01:%d, bit23:%d, bit45:%d, bit67:%d\n", pu.bit01, pu.bit23, pu.bit45, pu.bit67);
    dprint("palette_gp: %d\n", palette_group);
    */

    /*load bg rgb palette color*/
    palette_addr = palette_group * 4;
    pi = bg_palette_tbl_get(palette_addr++);
    palette_index_to_rgb15(pi, &plt->col[0]);
    /*
    dprint("palette 0: index:%02d, %02x %02x %02x\n", pi, 
            colto8bit(plt->col[0].r), colto8bit(plt->col[0].g), colto8bit(plt->col[0].b));
            */

    pi = bg_palette_tbl_get(palette_addr++);
    palette_index_to_rgb15(pi, &plt->col[1]);
    /*
    dprint("palette 1: index:%02d, %02x %02x %02x\n", pi, 
            colto8bit(plt->col[1].r), colto8bit(plt->col[1].g), colto8bit(plt->col[1].b));
            */

    pi = bg_palette_tbl_get(palette_addr++);
    palette_index_to_rgb15(pi, &plt->col[2]);
    /*
    dprint("palette 2: index:%02d, %02x %02x %02x\n", pi, 
            colto8bit(plt->col[2].r), colto8bit(plt->col[2].g), colto8bit(plt->col[2].b));
            */

    pi = bg_palette_tbl_get(palette_addr);
    palette_index_to_rgb15(pi, &plt->col[3]);
    /*
    dprint("palette 3: index:%02d, %02x %02x %02x\n", pi, 
            colto8bit(plt->col[3].r), colto8bit(plt->col[3].g), colto8bit(plt->col[3].b));
            */

}

void load_spr_attribute(struct sprite_attr sa, struct palette *plt) {
    unsigned short palette_addr;
    unsigned char pi;

    /*load bg rgb palette color*/
    palette_addr = sa.palette * 4;
    pi = bg_palette_tbl_get(palette_addr++);
    palette_index_to_rgb15(pi, &plt->col[0]);

    pi = bg_palette_tbl_get(palette_addr++);
    palette_index_to_rgb15(pi, &plt->col[1]);

    pi = bg_palette_tbl_get(palette_addr++);
    palette_index_to_rgb15(pi, &plt->col[2]);

    pi = bg_palette_tbl_get(palette_addr);
    palette_index_to_rgb15(pi, &plt->col[3]);
}

/*
 * pattern index: 0 - 255
 * */
void load_pattern(unsigned char bank, unsigned char ptn_index, struct tile_2* pattern) {
    int i;
    unsigned char *p;
    unsigned short addr;

    //load character pattern
    p = (unsigned char*)pattern;
    addr = ptn_index * sizeof(struct tile_2);
    for (i = 0; i < sizeof(struct tile_2); i++) {
        *p = pattern_tbl_get(bank, addr);
        p++;
        addr++;
    }
}

int load_chr_rom(FILE* cartridge, int num_rom_bank) {
    int len;

    len = fread(pattern_tbl0, 1, PATTERN_TBL_SIZE, cartridge);
    if (len != PATTERN_TBL_SIZE)
        return FALSE;

    len = fread(pattern_tbl1, 1, PATTERN_TBL_SIZE, cartridge);
    if (len != PATTERN_TBL_SIZE)
        return FALSE;

    return TRUE;
}

int vram_init(void) {
    name_tbl2 = NULL;
    name_tbl3 = NULL;

    attr_tbl2 = NULL;
    attr_tbl3 = NULL;
    

    pattern_tbl0 = malloc(PATTERN_TBL_SIZE);
    if (pattern_tbl0 == NULL)
        return FALSE;

    pattern_tbl1 = malloc(PATTERN_TBL_SIZE);
    if (pattern_tbl1 == NULL)
        return FALSE;

    sprite_ram = malloc(SPRITE_RAM_SIZE);
    if (sprite_ram == NULL)
        return FALSE;

    name_tbl0 = malloc(NAME_TBL_SIZE);
    if (name_tbl0 == NULL)
        return FALSE;

    name_tbl1 = malloc(NAME_TBL_SIZE);
    if (name_tbl1 == NULL)
        return FALSE;

    attr_tbl0 = malloc(ATTR_TBL_SIZE);
    if (attr_tbl0 == NULL)
        return FALSE;

    attr_tbl1 = malloc(ATTR_TBL_SIZE);
    if (attr_tbl1 == NULL)
        return FALSE;

    bg_palette_tbl = malloc(PALETTE_TBL_SIZE);
    if (bg_palette_tbl == NULL)
        return FALSE;

    spr_palette_tbl = malloc(PALETTE_TBL_SIZE);
    if (spr_palette_tbl == NULL)
        return FALSE;

    return TRUE;
}

void clean_vram(void) {

    free(pattern_tbl0);
    free(pattern_tbl1);

    free(sprite_ram);

    free(name_tbl0);
    free(name_tbl1);

    free(attr_tbl0);
    free(attr_tbl1);

    free(bg_palette_tbl);
    free(spr_palette_tbl);

}


/*
 * type 
 * 0: pattern table
 * 1: name table
 * 2: attribute table
 * 3: palette table (bank=0: bg, bank=1: sprite)
 * 4: sprite ram
 * */
void dump_vram(int type, int bank, unsigned short addr, int size) {
    char buf[100];
    unsigned short base;
    unsigned char *mem;

    switch(type) {
        case VRAM_DUMP_TYPE_PTN:
            sprintf(buf, "pattern table %d:\n", bank);
            base = (bank == 0 ? 0 : 0x1000);
            mem = (bank == 0 ? pattern_tbl0 : pattern_tbl1);
            break;

        case VRAM_DUMP_TYPE_NAME:
            sprintf(buf, "name table %d:\n", bank);
            base = 0x2000 + bank * 0x400;
            switch (bank) {
                case 0:
                    mem = name_tbl0;
                    break;
                case 1:
                    mem = name_tbl1;
                    break;
                case 2:
                    mem = name_tbl2;
                    break;
                case 3:
                default:
                    mem = name_tbl3;
                    break;
            }
            break;

        case VRAM_DUMP_TYPE_ATTR:
            sprintf(buf, "attribute table %d:\n", bank);
            base = 0x23c0 + bank * 0x400;
            switch (bank) {
                case 0:
                    mem = attr_tbl0;
                    break;
                case 1:
                    mem = attr_tbl1;
                    break;
                case 2:
                    mem = attr_tbl2;
                    break;
                case 3:
                default:
                    mem = attr_tbl3;
                    break;
            }
            break;

        case VRAM_DUMP_TYPE_PLT:
            switch (bank) {
                case 0:
                    base = 0x3f00;
                    sprintf(buf, "bg palette table %d:\n", bank);
                    mem = bg_palette_tbl;
                    break;
                case 1:
                default:
                    base = 0x3f10;
                    sprintf(buf, "sprite palette table %d:\n", bank);
                    mem = spr_palette_tbl;
                    break;
            }
            break;

        case VRAM_DUMP_TYPE_SPR:
        default:
            sprintf(buf, "sprite ram:\n");
            base = 0;
            mem = sprite_ram;
            break;

    }
    dump_mem(buf, base, addr, mem, size);
}


