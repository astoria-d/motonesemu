#include <string.h>
#include "tools.h"
#include "vga.h"
#include "ppucore.h"

void vscreenn_dot_get(int x, int y, struct rgb15 *col);
struct rgb15 *get_vscreen_head(void);

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

void vga_xfer_old(int scanline) {
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

static struct rgb15 *vs_col, *vs_head;
static struct rgb15 *vga_col;

void vga_posinit(void) {
    vs_head = (struct rgb15 *)get_vscreen_head();
}

void vga_xfer(int vs_x, int vs_y) {
    int vga_x, vga_y, vga_x_next;
    struct rgb15 *vga_col_next;

    //dprint("vga_xfer x:%d, y:%d\n", vs_x, vs_y);
    //x direction scale is 640/256.
    //y direction scale is 480/240

    if (vs_x == 0) {
        int tile_id, tile_id_y;
        int inner_y;

        tile_id_y = vs_y / TILE_DOT_SIZE;
        tile_id = tile_id_y * H_SCREEN_TILE_SIZE;
        inner_y = vs_y % TILE_DOT_SIZE;
            
        vs_col = vs_head + tile_id * TILE_DOT_SIZE * TILE_DOT_SIZE + inner_y * TILE_DOT_SIZE;

        vga_y = vs_y * VGA_HEIGHT / VSCREEN_HEIGHT;
        vga_col = vga_base + vga_y * VGA_WIDTH;
    }
    else {
        if (vs_x % TILE_DOT_SIZE) {
            vs_col++;
        }
        else {
            vs_col += (TILE_DOT_SIZE - 1) * TILE_DOT_SIZE + 1;
        }
    }

    vga_x = vs_x * VGA_WIDTH / VSCREEN_WIDTH;
    vga_x_next = (vs_x + 1) * VGA_WIDTH / VSCREEN_WIDTH;
    vga_col_next = vga_col + vga_x_next - vga_x;

    //copy color in vscreen to vga buffer.
    while ( vga_col != vga_col_next) {
        *vga_col = *vs_col;
        //double the copy of the next dot below as well 
        //since y direction scale is just x2.
        *(vga_col + VGA_WIDTH) = *vs_col;
        vga_col++;
    }

}

int vga_xfer_init(void) {
    emp_red = FALSE;
    emp_green = FALSE;
    emp_blue = FALSE;
    return TRUE;
}
