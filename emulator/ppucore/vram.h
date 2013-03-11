#ifndef __vram_h__
#define __vram_h__

#include "vga.h"
#include "ppucore.h"

unsigned char pattern_tbl_get(unsigned char bank, unsigned short addr);

unsigned char name_tbl_get(unsigned char bank, unsigned short addr);
void name_tbl_set(unsigned char bank, unsigned short addr, unsigned char data);

unsigned char attr_tbl_get(unsigned char bank, unsigned short addr);
void attr_tbl_set(unsigned char bank, unsigned short addr, unsigned char data);

unsigned char spr_plt_tbl_get(unsigned short addr);
void spr_plt_tbl_set(unsigned short addr, unsigned char data);


unsigned char spr_palette_tbl_get(unsigned short addr);
void spr_palette_tbl_set(unsigned short addr, unsigned char data);

unsigned char img_palette_tbl_get(unsigned short addr);
void img_palette_tbl_set(unsigned short addr, unsigned char data);

int vram_init(void);
void clean_vram(void);

struct tile_1_line{
    unsigned int dot0   :1;
    unsigned int dot1   :1;
    unsigned int dot2   :1;
    unsigned int dot3   :1;
    unsigned int dot4   :1;
    unsigned int dot5   :1;
    unsigned int dot6   :1;
    unsigned int dot7   :1;
} __attribute__ ((packed));

struct tile_1 {
    struct tile_1_line l[TILE_DOT_SIZE];
};

struct tile_2 {
    struct tile_1 b0;
    struct tile_1 b1;
};

struct palette {
    struct rgb15 col[4];
};

struct palette_unit {
    unsigned int    bit01   :2;
    unsigned int    bit23   :2;
    unsigned int    bit45   :2;
    unsigned int    bit67   :2;
} __attribute__ ((packed));

#endif /*__vram_h__*/

