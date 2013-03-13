#ifndef __vga_xfer_h__
#define __vga_xfer_h__


int vga_xfer_init(void);
void set_emphasize_red(int set);
void set_emphasize_green(int set);
void set_emphasize_blue(int set);

void show_leftside_sprite(void);
void show_leftside_bg(void);

#endif /*__vga_xfer_h__*/

