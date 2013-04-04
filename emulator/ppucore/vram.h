#ifndef __vram_h__
#define __vram_h__

#include "vga.h"
#include "ppucore.h"

unsigned char vram_data_get(unsigned short addr);
void vram_data_set(unsigned short addr, unsigned char data);

int init_vram(void);
void clean_vram(void);

/*
 * NES is little endian.
 * low bit comes first.
 * high bit follows.
 *
 * */
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
    struct tile_1 b1;
    struct tile_1 b0;
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

#define colto5bit(col8) ((col8) * 0x1F / 0xFF)
#define colto8bit(col5) (((unsigned int)(col5)) * 0xFF / 0x1F)

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
#define NAME_TBL_SIZE       (V_SCREEN_TILE_SIZE * H_SCREEN_TILE_SIZE)
#define ATTR_TBL_SIZE       (VIRT_SCREEN_TILE_SIZE * VIRT_SCREEN_TILE_SIZE \
                            / ATTR_GROUP_UNIT / ATTR_UNIT_PER_BYTE)
#define PALETTE_TBL_SIZE    0x10

#define PATTERN_ADDR_MASK       (PATTERN_TBL_SIZE - 1)
#define ATTR_TBL_ADDR_MASK      (ATTR_TBL_SIZE - 1)
#define PALETTE_TBL_ADDR_MASK   (PALETTE_TBL_SIZE - 1)

#define PPU_ADDR_MASK       (0x4000 - 1)
#define PALETTE_START       0x3F00
#define NAME_ATTR_MASK      0x2FFF
#define PALETTE_SPRITE_BIT  0x10

#define NAME0_START         (PATTERN_TBL_SIZE * 2)
#define ATTR0_START         (NAME0_START + NAME_TBL_SIZE)
#define NAME1_START         (ATTR0_START + ATTR_TBL_SIZE)
#define ATTR1_START         (NAME1_START + NAME_TBL_SIZE)
#define NAME2_START         (ATTR1_START + ATTR_TBL_SIZE)
#define ATTR2_START         (NAME2_START + NAME_TBL_SIZE)
#define NAME3_START         (ATTR2_START + ATTR_TBL_SIZE)
#define ATTR3_START         (NAME3_START + NAME_TBL_SIZE)

#endif /*__vram_h__*/

