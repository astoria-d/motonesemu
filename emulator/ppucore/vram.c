#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "tools.h"
#include "ppucore.h"
#include "vram.h"
#include "sprite.h"

void palette_index_to_rgb15(unsigned char index, struct rgb15* rgb);
void dump_mem(const char* msg, unsigned short base, 
        unsigned short offset, unsigned char* buf, int size);
void set_bgtile(int tile_id);

/*vram definition*/

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

//#define PPU_TEST
#ifdef PPU_TEST
static int first_time = TRUE;
#endif /* PPU_TEST */

extern int debug_mode;
static void (*vram_log_level4)(unsigned short, unsigned char);
static void dump_vram_write (unsigned short addr, unsigned char data);
void d4_set(int on_off);

/*
 * VRAM get/set functions....
 *
 * */

static unsigned char pattern_tbl_get(unsigned char bank, unsigned short offset) {
    //bank is masked to fit within 0-1
    bank &= 0x01;
    if (bank == 0)
        return pattern_tbl0[offset];
    else
        return pattern_tbl1[offset];
}

static unsigned char name_tbl_get(unsigned char bank, unsigned short offset) {
    //bank is masked to fit within 0-3
    bank &= 0x03;
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
    //bank is masked to fit within 0-3
    bank &= 0x03;
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
    //bank is masked to fit within 0-3
    bank &= 0x03;
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
    //bank is masked to fit within 0-3
    bank &= 0x03;
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

void vram_data_set(unsigned short addr, unsigned char data) {
#ifdef PPU_TEST
    if (!first_time)
        return;
#endif /* PPU_TEST */

    vram_log_level4 (addr, data);

    //mirror 0x4000 up addr.
    addr &= PPU_ADDR_MASK;

    if (addr < 2 * PATTERN_TBL_SIZE) {
        //dprint("invalid vram write addr:%04x, data:%2x\n", addr, data);
        //do nothing. pattern table is read only.
        extern int critical_error;
        dprint("invalid vram write!!!!\n");
        critical_error = TRUE;
    }
    else if (addr >= PALETTE_START) {
        // bg/sprite palette table.
        if (addr & PALETTE_SPRITE_BIT)
            spr_palette_tbl_set(addr & PALETTE_TBL_ADDR_MASK, data);
        else 
            bg_palette_tbl_set(addr & PALETTE_TBL_ADDR_MASK, data);
    }
    else {
        // name/attr table.
        // mask 0x3000 up addr.
        addr &= NAME_ATTR_MASK;
        if (addr < ATTR0_START) {
            name_tbl_set(0, addr - NAME0_START, data);
        }
        else if (addr < NAME1_START) {
            attr_tbl_set(0, addr - ATTR0_START, data);
        }
        else if (addr < ATTR1_START) {
            name_tbl_set(1, addr - NAME1_START, data);
        }
        else if (addr < NAME2_START) {
            attr_tbl_set(1, addr - ATTR1_START, data);
        }
        else if (addr < ATTR2_START) {
            name_tbl_set(2, addr - NAME2_START, data);
        }
        else if (addr < NAME3_START) {
            attr_tbl_set(2, addr - ATTR2_START, data);
        }
        else if (addr < ATTR3_START) {
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
        return pattern_tbl_get(0, addr & PATTERN_ADDR_MASK);
    }
    if (addr < 2 * PATTERN_TBL_SIZE) {
        return pattern_tbl_get(1, addr & PATTERN_ADDR_MASK);
    }
    else if (addr >= PALETTE_START) {
        if (addr & PALETTE_SPRITE_BIT)
            return spr_palette_tbl_get(addr & PALETTE_TBL_ADDR_MASK);
        else 
            return bg_palette_tbl_get(addr & PALETTE_TBL_ADDR_MASK);
    }
    else {
        addr &= NAME_ATTR_MASK;
        if (addr < ATTR0_START) {
            return name_tbl_get(0, addr - NAME0_START);
        }
        else if (addr < NAME1_START) {
            return attr_tbl_get(0, addr - ATTR0_START);
        }
        else if (addr < ATTR1_START) {
            return name_tbl_get(1, addr - NAME1_START);
        }
        else if (addr < NAME2_START) {
            return attr_tbl_get(1, addr - ATTR1_START);
        }
        else if (addr < ATTR2_START) {
            return name_tbl_get(2, addr - NAME2_START);
        }
        else if (addr < NAME3_START) {
            return attr_tbl_get(2, addr - ATTR2_START);
        }
        else if (addr < ATTR3_START) {
            return name_tbl_get(3, addr - NAME3_START);
        }
        else {
            return attr_tbl_get(3, addr - ATTR3_START);
        }
    }
    return 0;
}

/* VRAM manipulation... */

#ifdef PPU_TEST
/*
 * ppu test function
 * */
static void test_ppu(void) {
    int i;
    unsigned char plt[32] = {
            0x0f, 0x00, 0x10, 0x20,
            0x0f, 0x06, 0x16, 0x26,
            0x0f, 0x08, 0x18, 0x28,
            0x0f, 0x0a, 0x1a, 0x2a,

            0x0f, 0x00, 0x10, 0x20,
            0x0f, 0x06, 0x16, 0x26,
            0x0f, 0x08, 0x18, 0x28,
            0x0f, 0x0a, 0x1a, 0x2a,
    };

    //bg character base addr set to 0x1000.
    ppu_ctrl1_set(0x00);

/*
*/
    //palette tbl
    for (i = 0; i < 16; i++)
        vram_data_set(0x3f00 + i, plt[i]);
    for (i = 0; i < 16; i++)
        vram_data_set(0x3f10 + i, plt[i + 16]);

    //name tbl
    for (i = 0; i < 960; i++) 
        vram_data_set(0x2000 + i, i %255);

/*
*/
    //attr tbl
    for (i = 0; i < 64; i++) 
        vram_data_set(0x23c0 + i, i%16);

    /*
    vram_data_set(0x2000 + 205, 'D');
    vram_data_set(0x2000 + 206, 'e');
    vram_data_set(0x2000 + 207, 'e');
    vram_data_set(0x2000 + 208, '!');
    vram_data_set(0x2000 + 209, '!');
    //205 = palette gp2 00011011b 
    //205 = 11
    vram_data_set(0x23c0 + 11, 0x1b);

    //other test.
    vram_data_set(0x2000 + 300, 1);
    vram_data_set(0x2000 + 0, 0x65);
*/

    /*
    set_monocolor(FALSE);
     * */

    for (i = 0; i < 960; i++) 
        set_bgtile(i);

    /*
    //sprite test
    struct sprite_attr sa;
    sa.palette = 2;
    sa.priority = 1;
    sa.flip_h = 0;
    sa.flip_v = 0;
    set_sprite(30, 100, 'd', sa);
    sa.flip_h = 1;
    set_sprite(50, 100, 'd', sa);
    sa.flip_v = 1;
    set_sprite(70, 105, 'd', sa);
     * */

    //bg&sprite show
    ppu_ctrl2_set(0x18);

    /*
    vga_xfer();
*/

//void dump_vram(int type, int bank, unsigned short addr, int size);
/*
    dump_vram(VRAM_DUMP_TYPE_PTN, 0, 0, 0x100);
    dump_vram(VRAM_DUMP_TYPE_NAME, 0, 0, 300);
    dump_vram(VRAM_DUMP_TYPE_ATTR, 0, 0, 64);
    dump_vram(VRAM_DUMP_TYPE_PLT, 0, 0, 16);
*/
}
#endif /* PPU_TEST */

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

void load_spr_palette(struct sprite_attr sa, struct palette *plt) {
    unsigned short palette_addr;
    unsigned char pi;

    /*load bg rgb palette color*/
    palette_addr = sa.palette * 4;
    pi = spr_palette_tbl_get(palette_addr++);
    palette_index_to_rgb15(pi, &plt->col[0]);

    pi = spr_palette_tbl_get(palette_addr++);
    palette_index_to_rgb15(pi, &plt->col[1]);

    pi = spr_palette_tbl_get(palette_addr++);
    palette_index_to_rgb15(pi, &plt->col[2]);

    pi = spr_palette_tbl_get(palette_addr);
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

    pattern_tbl0 = malloc(PATTERN_TBL_SIZE);
    if (pattern_tbl0 == NULL)
        return FALSE;

    pattern_tbl1 = malloc(PATTERN_TBL_SIZE);
    if (pattern_tbl1 == NULL)
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

    name_tbl2 = name_tbl0;
    name_tbl3 = name_tbl1;

    attr_tbl2 = attr_tbl0;
    attr_tbl3 = attr_tbl1;
    
    bg_palette_tbl = malloc(PALETTE_TBL_SIZE);
    if (bg_palette_tbl == NULL)
        return FALSE;

    spr_palette_tbl = malloc(PALETTE_TBL_SIZE);
    if (spr_palette_tbl == NULL)
        return FALSE;

    memset(pattern_tbl0, 0, PATTERN_TBL_SIZE);
    memset(pattern_tbl1, 0, PATTERN_TBL_SIZE);
    memset(name_tbl0, 0, NAME_TBL_SIZE);
    memset(name_tbl1, 0, NAME_TBL_SIZE);
    memset(attr_tbl0, 0, ATTR_TBL_SIZE);
    memset(attr_tbl1, 0, ATTR_TBL_SIZE);
    memset(bg_palette_tbl, 0, PALETTE_TBL_SIZE);
    memset(spr_palette_tbl, 0, PALETTE_TBL_SIZE);

    //d4_set(debug_mode);
    d4_set(FALSE);

    return TRUE;
}

void clean_vram(void) {

    free(pattern_tbl0);
    free(pattern_tbl1);

    free(name_tbl0);
    free(name_tbl1);

    free(attr_tbl0);
    free(attr_tbl1);

    free(bg_palette_tbl);
    free(spr_palette_tbl);

}

static void null_write (unsigned short addr, unsigned char data) {}
static void dump_vram_write (unsigned short addr, unsigned char data) {
    dprint("                                  ");
    dprint("vram_data_set addr:%04x, data:%2x\n", addr, data);
}

void d4_set(int on_off) {
    if (on_off) {
        vram_log_level4 = dump_vram_write;
    }
    else {
        vram_log_level4 = null_write;
    }
}
