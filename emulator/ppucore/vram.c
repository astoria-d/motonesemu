#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tools.h"
#include "ppucore.h"
#include "vram.h"

void palette_index_to_rgb15(int bank, unsigned char index, struct rgb15* rgb);
void dump_mem(const char* msg, unsigned short base, 
        unsigned short offset, unsigned char* buf, int size);

#define PATTERN_TBL_SIZE    0x1000
#define NAME_TBL_SIZE       V_SCREEN_TILE_SIZE * H_SCREEN_TILE_SIZE
#define ATTR_TBL_SIZE       (VIRT_SCREEN_TILE_SIZE * VIRT_SCREEN_TILE_SIZE \
                            / ATTR_GROUP_UNIT / ATTR_UNIT_PER_BYTE)
#define PALETTE_TBL_SIZE    16
#define SPRITE_RAM_SIZE     0xff

#define PATTERN_ADDR_MASK       (PATTERN_TBL_SIZE - 1)
#define NAME_TBL_ADDR_MASK      (NAME_TBL_SIZE - 1)
#define ATTR_TBL_ADDR_MASK      (ATTR_TBL_SIZE - 1)
#define PALETTE_TBL_ADDR_MASK   (PALETTE_TBL_SIZE - 1)
#define SPR_RAM_ADDR_MASK       (SPRITE_RAM_SIZE - 1)

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

unsigned char pattern_tbl_get(unsigned char bank, unsigned short addr) {
    addr = addr & PATTERN_ADDR_MASK;
    if (bank == 0)
        return pattern_tbl0[addr];
    else
        return pattern_tbl1[addr];
}

unsigned char name_tbl_get(unsigned char bank, unsigned short addr) {
    addr = addr & NAME_TBL_ADDR_MASK;
    if (bank == 0)
        return name_tbl0[addr];
    else if (bank == 1)
        return name_tbl1[addr];
    else if (bank == 2)
        return name_tbl2[addr];
    else
        return name_tbl3[addr];
}

void name_tbl_set(unsigned char bank, unsigned short addr, unsigned char data) {
    addr = addr & NAME_TBL_ADDR_MASK;
    if (bank == 0)
        name_tbl0[addr] = data;
    else if (bank == 1)
        name_tbl1[addr] = data;
    else if (bank == 2)
        name_tbl2[addr] = data;
    else
        name_tbl3[addr] = data;
}


unsigned char attr_tbl_get(unsigned char bank, unsigned short addr) {
    addr = addr & ATTR_TBL_ADDR_MASK;
    if (bank == 0)
        return attr_tbl0[addr];
    else if (bank == 1)
        return attr_tbl1[addr];
    else if (bank == 2)
        return attr_tbl2[addr];
    else
        return attr_tbl3[addr];
}

void attr_tbl_set(unsigned char bank, unsigned short addr, unsigned char data) {
    addr = addr & ATTR_TBL_ADDR_MASK;
    if (bank == 0)
        attr_tbl0[addr] = data;
    else if (bank == 1)
        attr_tbl1[addr] = data;
    else if (bank == 2)
        attr_tbl2[addr] = data;
    else
        attr_tbl3[addr] = data;
}


unsigned char spr_palette_tbl_get(unsigned short addr) {
    addr = addr & PALETTE_TBL_ADDR_MASK;
    return spr_palette_tbl[addr];
}

void spr_palette_tbl_set(unsigned short addr, unsigned char data) {
    addr = addr & PALETTE_TBL_ADDR_MASK;
    spr_palette_tbl[addr] = data;
}

unsigned char bg_palette_tbl_get(unsigned short addr) {
    addr = addr & PALETTE_TBL_ADDR_MASK;
    return bg_palette_tbl[addr];
}

void bg_palette_tbl_set(unsigned short addr, unsigned char data) {
    addr = addr & PALETTE_TBL_ADDR_MASK;
    bg_palette_tbl[addr] = data;
}


unsigned char spr_ram_tbl_get(unsigned short addr) {
    addr = addr & SPR_RAM_ADDR_MASK;
    return sprite_ram[addr];
}

void spr_ram_tbl_set(unsigned short addr, unsigned char data) {
    addr = addr & SPR_RAM_ADDR_MASK;
    sprite_ram[addr] = data;
}

/* VRAM manipulation... */

void load_attribute(unsigned char bank, int tile_index, struct palette *plt) {
    int gp_index;
    int unit_index;
    unsigned char data;
    struct palette_unit pu;
    int palette_group;
    unsigned short palette_addr;
    unsigned char pi;

    gp_index = tile_index / ATTR_GROUP_UNIT / ATTR_UNIT_PER_BYTE;
    unit_index = tile_index / ATTR_GROUP_UNIT;
    data = attr_tbl_get(bank, gp_index);
    pu = *(struct palette_unit*)&data;
    memcpy(&pu, &data, sizeof(pu));
    //dprint("attr data:%1x, pu size:%d\n", data, sizeof(pu));

    switch(unit_index) {
        case 0:
            palette_group = pu.bit01;
            break;
        case 1:
            palette_group = pu.bit23;
            break;
        case 2:
            palette_group = pu.bit45;
            break;
        default:
            palette_group = pu.bit67;
            break;
    }

    /*load bg rgb palette color*/
    palette_addr = palette_group * 4;
    pi = bg_palette_tbl_get(palette_addr++);
    palette_index_to_rgb15(0, pi, &plt->col[0]);
    pi = bg_palette_tbl_get(palette_addr++);
    palette_index_to_rgb15(0, pi, &plt->col[1]);
    pi = bg_palette_tbl_get(palette_addr++);
    palette_index_to_rgb15(0, pi, &plt->col[2]);
    pi = bg_palette_tbl_get(palette_addr);
    palette_index_to_rgb15(0, pi, &plt->col[3]);

}

/*
 * pattern index: 0 - 255
 * */
void load_pattern(unsigned char bank, unsigned char ptn_index, struct tile_2* pattern) {
    int i;
    unsigned char data;
    unsigned char *p;
    unsigned short addr;

    //load character pattern
    p = (unsigned char*)pattern;
    addr = ptn_index * sizeof(struct tile_2);
    for (i = 0; i < sizeof(struct tile_2); i++) {
        data = pattern_tbl_get(bank, addr);
        *p = data;
        p++;
    }
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
        case 0:
            sprintf(buf, "pattern table %d:\n", bank);
            base = (bank == 0 ? 0 : 0x1000);
            mem = (bank == 0 ? pattern_tbl0 : pattern_tbl1);
            break;

        case 1:
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

        case 2:
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

        case 3:
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

        case 4:
        default:
            sprintf(buf, "sprite ram:\n");
            base = 0;
            mem = sprite_ram;
            break;

    }
    dump_mem(buf, base, addr, mem, size);
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


