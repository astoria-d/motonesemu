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
    unsigned int priority   :1;
    unsigned int flip_h     :1;
    unsigned int flip_v     :1;
} __attribute__ ((packed));

#endif /*__sprite_h__*/

