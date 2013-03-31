#ifndef __sprite_h__
#define __sprite_h__

/*
 * NES is little endian.
 * low bit comes first.
 * high bit follows.
 * */
struct sprite_attr {
    unsigned int palette    :2;
    unsigned int unused     :3;
    unsigned int priority   :1;     /*0: foreground, 1:background.*/
    unsigned int flip_h     :1;
    unsigned int flip_v     :1;
} __attribute__ ((packed));

struct ppu_sprite_reg {
    unsigned char y;
    unsigned char index;
    struct sprite_attr sa;
    unsigned char x;
    unsigned int cnt        :2;
};

#define SPRITE_RAM_SIZE         0x100
#define SPRITE_CNT              (SPRITE_RAM_SIZE / 4)
#define SPR_RAM_ADDR_MASK       (SPRITE_RAM_SIZE - 1)

int sprite_init(void);
void clean_sprite(void);

#endif /*__sprite_h__*/

