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

static int vga_y;
static int vscrn_y, vscrn_y_old;
static struct rgb15 *vga_col;

void vga_xfer(int scanline) {
    int vscrn_x, vscrn_x_old;
    int vga_x;
    struct rgb15 *col_old;

    if (scanline == 0) {
        vga_y = 0;
        vscrn_y_old = -1;
        vga_col = vga_base;
    }

    vscrn_x_old = -1;
    col_old = vga_col;
    while (1) {
        vscrn_y = vga_y * VSCREEN_HEIGHT / VGA_HEIGHT;
        if (vscrn_y != scanline)
            break;

        if (vscrn_y != vscrn_y_old) {
            for (vga_x = 0; vga_x < VGA_WIDTH; vga_x++) {
                vscrn_x = vga_x * VSCREEN_WIDTH / VGA_WIDTH;
                if (vscrn_x != vscrn_x_old) {
                    vscreenn_dot_get(vscrn_x, vscrn_y, vga_col);
                }
                else {
                    *vga_col = *col_old;
                }

                vscrn_x_old = vscrn_x;
                col_old = vga_col;
                vga_col++;
            }
        }
        else {
            memcpy(vga_col, col_old, VGA_WIDTH * sizeof (struct rgb15));
            col_old = vga_col;
            vga_col += VGA_WIDTH;
        }

        vga_y++;
        vscrn_y_old = vscrn_y;
    }
}

int vga_xfer_init(void) {
    emp_red = FALSE;
    emp_green = FALSE;
    emp_blue = FALSE;
    return TRUE;
}
