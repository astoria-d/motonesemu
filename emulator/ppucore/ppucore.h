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

#endif /*__ppucore_h__*/

