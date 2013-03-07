
#define VGA_FIFO    "vga-comm"

#define VGA_WIDTH   640
#define VGA_HEIGHT  480

struct vga_pulse {
    //vertical sync bit
    unsigned int v_sync  :1;

    //horizontal sync bit
    unsigned int h_sync  :1;

    //rgb 15bit
    unsigned int r       :5;
    unsigned int g       :5;
    unsigned int b       :5;

};

#define to5bit(col16) col16 * 0x1F / 0xFFFF
#define to16bit(col5) col5 * 0xFFFF / 0x1F

