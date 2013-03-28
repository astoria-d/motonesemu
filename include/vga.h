#ifndef __vga_h__
#define __vga_h__

#define VGA_WIDTH   640
#define VGA_HEIGHT  480

/*
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
#define DISPLAY_PORT    9999
*/
struct rgb15 {
    unsigned int r   :5;
    unsigned int g   :5;
    unsigned int b   :5;
};


#define to5bit(col16) col16 * 0x1F / 0xFFFF
#define to16bit(col5) col5 * 0xFFFF / 0x1F


#define VGA_SHM             "vgadisp"
#define VGA_SHM_PRJ_ID      'm'
#define VGA_SHM_SIZE        (VGA_WIDTH * VGA_HEIGHT * sizeof (struct rgb15))

#define VGA_REFRESH_RATE    60

#endif /*__vga_h__*/
