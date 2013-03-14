#ifndef __ppucore_h__
#define __ppucore_h__

/*8x8 pixel is the character unit*/
#define TILE_DOT_SIZE  8

/*screen has 32x32*/
#define VIRT_SCREEN_TILE_SIZE   32
#define H_SCREEN_TILE_SIZE      32
#define V_SCREEN_TILE_SIZE      30

/*attribute table data has 16 tiles*/
#define ATTR_GROUP_UNIT         4
#define ATTR_UNIT_PER_BYTE      4


void ppu_ctrl1_set(unsigned char data);
void ppu_ctrl2_set(unsigned char data);
void sprite_addr_set(unsigned char data);
void sprite_data_set(unsigned char data);
void ppu_scroll_set(unsigned char data);
void ppu_vram_addr_set(unsigned char data);
void ppu_vram_data_set(unsigned char data);

unsigned char ppu_status_get(void);
unsigned char ppu_vram_data_get(void);

int ppucore_init(void);
void clean_ppucore(void);

#endif /*__ppucore_h__*/

