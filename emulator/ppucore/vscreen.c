#include <stdlib.h>
#include <string.h>

#include "libio.h"
#include "tools.h"
#include "vga.h"
#include "ppucore.h"
#include "vram.h"
#include "sprite.h"

void load_attribute(unsigned char bank, int tile_index, struct palette *plt);
void load_pattern(unsigned char bank, unsigned char ptn_index, struct tile_2* pattern);
void load_spr_attribute(struct sprite_attr sa, struct palette *plt);

struct tile_rgb15_line {
    struct rgb15 d[8];
};

struct tile_rgb15 {
    struct tile_rgb15_line l[8];
};

static struct tile_rgb15 *vscreen;

void vscreenn_dot_get(int x, int y, struct rgb15 *col) {
    int tile_id, tile_id_x, tile_id_y;
    int inner_x, inner_y;
    struct tile_rgb15* tile;

    tile_id_x = x / TILE_DOT_SIZE;
    tile_id_y = y / TILE_DOT_SIZE;
    tile_id = tile_id_x + tile_id_y * H_SCREEN_TILE_SIZE;
    tile = vscreen + tile_id;

    inner_x = x % TILE_DOT_SIZE;
    inner_y = y % TILE_DOT_SIZE;
    *col = tile->l[inner_y].d[inner_x];
}

void vscreenn_dot_set(int x, int y, struct rgb15 *col) {
    int tile_id, tile_id_x, tile_id_y;
    int inner_x, inner_y;
    struct tile_rgb15* tile;

    tile_id_x = x / TILE_DOT_SIZE;
    tile_id_y = y / TILE_DOT_SIZE;
    tile_id = tile_id_x + tile_id_y * H_SCREEN_TILE_SIZE;
    tile = vscreen + tile_id;

    inner_x = x % TILE_DOT_SIZE;
    inner_y = y % TILE_DOT_SIZE;
    tile->l[inner_y].d[inner_x] = *col;
}

static int pal_index(struct tile_2 *ptn, int l, int dot_x) {
    switch (dot_x) {
        case 0:
            return ptn->b0.l[l].dot0 * 2 + ptn->b1.l[l].dot0;
        case 1:
            return ptn->b0.l[l].dot1 * 2 + ptn->b1.l[l].dot1;
        case 2:
            return ptn->b0.l[l].dot2 * 2 + ptn->b1.l[l].dot2;
        case 3:
            return ptn->b0.l[l].dot3 * 2 + ptn->b1.l[l].dot3;
        case 4:
            return ptn->b0.l[l].dot4 * 2 + ptn->b1.l[l].dot4;
        case 5:
            return ptn->b0.l[l].dot5 * 2 + ptn->b1.l[l].dot5;
        case 6:
            return ptn->b0.l[l].dot6 * 2 + ptn->b1.l[l].dot6;
        case 7:
        default:
            return ptn->b0.l[l].dot7 * 2 + ptn->b1.l[l].dot7;
    }
}

void set_bgtile(int tile_id) {
    struct palette plt;
    struct tile_2 ptn;
    unsigned char name_index;
    struct tile_rgb15* set_data;
    int i,j;

    load_attribute(0, tile_id, &plt);

    name_index = name_tbl_get(0, tile_id);
    load_pattern(0, name_index, &ptn);

    set_data = vscreen + tile_id;
    for (i = 0; i < TILE_DOT_SIZE; i++) {
        //display shows left to right with high bit to low bit
        for (j = 0; j < 8; j++) {
            int pi = pal_index(&ptn, i, j);
            if (pi) {
                //dprint("%d, %d, colind:%d\n", j, i, pi);
                set_data->l[i].d[7 - j] = plt.col[pi];
            }
        }
    }

}

void set_sprite(int x, int y, int tile_id, struct sprite_attr sa) {
    struct palette plt;
    struct tile_2 ptn;
    int i, j;

    load_spr_attribute(sa, &plt);

    load_pattern(0, tile_id, &ptn);

    //display shows left to right with high bit to low bit
    for (i = 0; i < TILE_DOT_SIZE; i++) {
        if (sa.flip_h) {
            if (sa.flip_v) {
                for (j = 0; j < 8; j++) {
                    int pi = pal_index(&ptn, i, j);
                    if (pi)
                        vscreenn_dot_set(x + j, y + 7 - i, &plt.col[pi]);
                }
            }
            else {
                for (j = 0; j < 8; j++) {
                    int pi = pal_index(&ptn, i, j);
                    if (pi)
                        vscreenn_dot_set(x + j, y + i, &plt.col[pi]);
                }
            }
        }
        else {
            if (sa.flip_v) {
                for (j = 0; j < 8; j++) {
                    int pi = pal_index(&ptn, i, j);
                    if (pi)
                        vscreenn_dot_set(x + 7 - j, y + 7 - i, &plt.col[pi]);
                }
            }
            else {
                for (j = 0; j < 8; j++) {
                    int pi = pal_index(&ptn, i, j);
                    if (pi)
                        vscreenn_dot_set(x + 7 - j, y + i, &plt.col[pi]);
                }
            }
        }
    }
}

int vscreen_init(void) {
    vscreen = (struct tile_rgb15 *) malloc(
        sizeof (struct tile_rgb15) * VIRT_SCREEN_TILE_SIZE * VIRT_SCREEN_TILE_SIZE);
    if (vscreen == NULL)
        return FALSE;
    memset(vscreen, 0, sizeof (struct tile_rgb15) * VIRT_SCREEN_TILE_SIZE * VIRT_SCREEN_TILE_SIZE);

    //dprint("tile_1_line:%d tile_2 size:%d\n", sizeof(struct tile_1_line), sizeof(struct tile_2));

    return TRUE;
}

void clean_vscreen(void) {
    free(vscreen);
}

