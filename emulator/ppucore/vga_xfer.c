#include <string.h>
#include "tools.h"
#include "vga.h"
#include "ppucore.h"

void vscreenn_dot_get(int x, int y, struct rgb15 *col);

#define VSCREEN_WIDTH       (H_SCREEN_TILE_SIZE * TILE_DOT_SIZE)
#define VSCREEN_HEIGHT      (V_SCREEN_TILE_SIZE * TILE_DOT_SIZE)

#define MAX_5_BIT       0x1f
#define EMPHASIZE_MAX   0x1c
/*emphasize 10% increase.*/
#define EMPHASIZE_RATE  110

static struct rgb15 *vga_base;

static int emp_red;
static int emp_green;
static int emp_blue;

/*
 * at this moment PPU emphasize r/g/b feature is not supported.
 * TODO: emphasize color
 * */
void set_emphasize_red(int set) {
    emp_red = set;
}
void set_emphasize_green(int set) {
    emp_green = set;
}
void set_emphasize_blue(int set) {
    emp_blue = set;
}

void set_vga_base(unsigned char* base) {
    vga_base = (struct rgb15*)base;
}

/*
 * show left 8 pixels of sprite/background function not supported.
 * TODO: show left 8 pixels of sprite/bg
 * */
void show_leftside_sprite(void) {
}

void show_leftside_bg(void) {
}

void vga_xfer(void) {
    int vscrn_x, vscrn_y, vscrn_x_old, vscrn_y_old;
    int vga_x, vga_y;
    struct rgb15 *col, *col_old;

    col = vga_base;
    vscrn_x_old = vscrn_y_old = -1;
    for (vga_y = 0; vga_y < VGA_HEIGHT; vga_y++) {
        vscrn_y = vga_y * VSCREEN_HEIGHT / VGA_HEIGHT;

        if (vscrn_y != vscrn_y_old) {
            for (vga_x = 0; vga_x < VGA_WIDTH; vga_x++) {
                vscrn_x = vga_x * VSCREEN_WIDTH / VGA_WIDTH;
                if (vscrn_x != vscrn_x_old) {
                    vscreenn_dot_get(vscrn_x, vscrn_y, col);
                }
                else {
                    *col = *col_old;
                }

                vscrn_x_old = vscrn_x;
                col_old = col;
                col++;
            }
        }
        else {
            memcpy(col, col_old, VGA_WIDTH * sizeof (struct rgb15));
            col_old = col;
            col += VGA_WIDTH;
        }

        vscrn_y_old = vscrn_y;
    }
}

int vga_xfer_init(void) {
    emp_red = FALSE;
    emp_green = FALSE;
    emp_blue = FALSE;
    return TRUE;
}
