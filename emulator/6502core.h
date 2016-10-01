#ifndef __6502core_h__
#define __6502core_h__

#define ADDR_MODE_ZP        0
#define ADDR_MODE_ZP_X      1
#define ADDR_MODE_ZP_Y      2
#define ADDR_MODE_ABS       3
#define ADDR_MODE_ABS_X     4
#define ADDR_MODE_ABS_Y     5
#define ADDR_MODE_IND       6
#define ADDR_MODE_IMP       7
#define ADDR_MODE_ACC       8
#define ADDR_MODE_IMM       9
#define ADDR_MODE_REL       10
#define ADDR_MODE_INDEX_INDIR       11
#define ADDR_MODE_INDIR_INDEX       12

/*
 * 6502 little endian
 * hi bit > low bit order
 * but gcc generates low > hi order for bit field
 * */
struct status_reg {
    unsigned int carry          :1;
    unsigned int zero           :1;
    unsigned int irq_disable    :1;
    unsigned int decimal        :1;
    unsigned int break_mode     :1;
    unsigned int researved      :1;
    unsigned int overflow       :1;
    unsigned int negative       :1;
} __attribute__ ((packed));

struct cpu_6502 {
    unsigned char acc;
    unsigned char x;
    unsigned char y;
    unsigned char sp;
    struct status_reg status;
    unsigned short pc;
};

#endif /*__6502core_h__*/

