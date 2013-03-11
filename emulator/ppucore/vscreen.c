#include <stdlib.h>
#include <string.h>

#include "libio.h"
#include "tools.h"
#include "vga.h"
#include "ppucore.h"
#include "vram.h"

void load_attribute(unsigned char bank, int tile_index, struct palette *plt);
void load_pattern(unsigned char bank, unsigned char ptn_index, struct tile_2* pattern);

struct tile_rgb15_line {
    struct rgb15 d[8];
};

struct tile_rgb15 {
    struct tile_rgb15_line l[8];
};

static struct tile_rgb15 *vscreen;


void set_bgtile(int tile_id) {
    struct palette plt;
    struct tile_2 ptn;
    unsigned char name_index;
    struct tile_rgb15* set_data;
    int i;

    load_attribute(0, tile_id, &plt);

    name_index = name_tbl_get(0, tile_id);
    load_pattern(0, name_index, &ptn);

    set_data = vscreen + tile_id;
    for (i = 0; i < TILE_DOT_SIZE; i++) {
        set_data->l[i].d[0] = plt.col[ptn.b0.l[i].dot0 + ptn.b1.l[i].dot0];
        set_data->l[i].d[1] = plt.col[ptn.b0.l[i].dot1 + ptn.b1.l[i].dot1];
        set_data->l[i].d[2] = plt.col[ptn.b0.l[i].dot2 + ptn.b1.l[i].dot2];
        set_data->l[i].d[3] = plt.col[ptn.b0.l[i].dot3 + ptn.b1.l[i].dot3];
        set_data->l[i].d[4] = plt.col[ptn.b0.l[i].dot4 + ptn.b1.l[i].dot4];
        set_data->l[i].d[5] = plt.col[ptn.b0.l[i].dot5 + ptn.b1.l[i].dot5];
        set_data->l[i].d[6] = plt.col[ptn.b0.l[i].dot6 + ptn.b1.l[i].dot6];
        set_data->l[i].d[7] = plt.col[ptn.b0.l[i].dot7 + ptn.b1.l[i].dot7];
    }

}

void set_bg(void) {
/*
    int i;
    //struct tile_rgb15* set_data = vscreen + tile_id;
    unsigned char data;
    unsigned char *p;
    struct tile_2 pattern;
    unsigned short addr;
    //load name tbl
    //name_tbl_get();

    //load character pattern
    p = (unsigned char*)&pattern;
    addr = tile_id * sizeof(struct tile_2);
    for (i = 0; i < sizeof(struct tile_2); i++) {
        data = pattern_tbl_get(bg_bank, addr);
        *p = data;
        p++;
    }

    //load attribute.
*/
}

void vscreenn_dot_get(int x, int y, struct rgb15 *col) {
    int tile_id, tile_id_x, tile_id_y;
    int inner_x, inner_y;
    struct tile_rgb15* tile;

    tile_id_x = x / TILE_DOT_SIZE;
    tile_id_y = y / TILE_DOT_SIZE;
    tile_id = tile_id_x + tile_id_y * V_SCREEN_TILE_SIZE;
    tile = vscreen + tile_id;

    inner_x = x % TILE_DOT_SIZE;
    inner_y = y % TILE_DOT_SIZE;
    *col = tile->l[inner_y].d[inner_x];

}

int vscreen_init(void) {
    vscreen = (struct tile_rgb15 *) malloc(
        sizeof (struct tile_rgb15) * VIRT_SCREEN_TILE_SIZE * VIRT_SCREEN_TILE_SIZE);
    if (vscreen == NULL)
        return FALSE;
    memset(vscreen, 0, sizeof (struct tile_rgb15) * VIRT_SCREEN_TILE_SIZE * VIRT_SCREEN_TILE_SIZE);

    dprint("tile_1_line:%d tile_2 size:%d\n", sizeof(struct tile_1_line), sizeof(struct tile_2));

    return TRUE;
}

void clean_vscreen(void) {
    free(vscreen);
}

