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
void sprite0_hit_set(void);
unsigned char spr_ram_tbl_get(unsigned short offset);
unsigned char vram_data_get(unsigned short addr);
void palette_index_to_rgb15(unsigned char index, struct rgb15* rgb);


#define TRANSPARENT_PALETTE_ADDR        0x3F10

struct tile_rgb15_line {
    struct rgb15 d[8];
};

struct tile_rgb15 {
    struct tile_rgb15_line l[8];
};

struct sprite_buf_reg {
    struct palette plt;
    struct tile_2 ptn;
};

static struct tile_rgb15 *vscreen;

static unsigned char bg_pattern_bank;
static unsigned char spr_pattern_bank;
static unsigned short   bg_name_tbl_base;
static unsigned char    bg_attr_tbl_bank;


#define SPRITE_PREFETCH_CNT     64
static struct ppu_sprite_reg sprite_temp_buf [SPRITE_PREFETCH_CNT];
static struct sprite_buf_reg sprite_buf [SPRITE_PREFETCH_CNT];
static int sprite_hit_cnt;


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

    load_attribute(bg_attr_tbl_bank, tile_id, &plt);

    name_index = vram_data_get(bg_name_tbl_base + tile_id);
    load_pattern(bg_pattern_bank, name_index, &ptn);

    set_data = vscreen + tile_id;
    for (i = 0; i < TILE_DOT_SIZE; i++) {
        //display shows left to right with high bit to low bit
        for (j = 0; j < 8; j++) {
            int pi = pal_index(&ptn, i, j);
            if (pi) {
                //dprint("%d, %d, colind:%d\n", j, i, pi);
                set_data->l[i].d[7 - j] = plt.col[pi];
            }
            else {
                //transparent bg color is read from sprite 0x10 color.
                pi = vram_data_get(TRANSPARENT_PALETTE_ADDR);
                palette_index_to_rgb15(pi, &set_data->l[i].d[7 - j]);
                /*
                set_data->l[i].d[7 - j].r = 0;
                set_data->l[i].d[7 - j].g = 0;
                set_data->l[i].d[7 - j].b = 0;
                */
            }
        }
    }

}

int load_background_old(int scanline) {
    int i, start, end;

    //load tile must be executed every 8 scanlines only.
    if (scanline % TILE_DOT_SIZE)
        return TRUE;

    start = scanline / TILE_DOT_SIZE * H_SCREEN_TILE_SIZE;
    end = start + H_SCREEN_TILE_SIZE;
    for (i = start; i < end; i++) {
        set_bgtile(i);
    }
    return TRUE;
}

static struct palette plt;
static struct tile_2 ptn;
static struct tile_rgb15* set_data;
int load_background(int x, int y) {
    //dprint("load bg x:%d, y:%d...\n", x, y);

    int inner_x, inner_y;


    //tile loading happens every 8 dots only.
    if (x % TILE_DOT_SIZE == 0) {
        int tile_id, tile_id_x, tile_id_y;
        unsigned char name_index;

        tile_id_x = x / TILE_DOT_SIZE;
        tile_id_y = y / TILE_DOT_SIZE;
        tile_id = tile_id_x + tile_id_y * H_SCREEN_TILE_SIZE;

        //dprint("load tile.\n");
        load_attribute(bg_attr_tbl_bank, tile_id, &plt);
        name_index = vram_data_get(bg_name_tbl_base + tile_id);
        load_pattern(bg_pattern_bank, name_index, &ptn);
        set_data = vscreen + tile_id;
    }


    inner_x = x % TILE_DOT_SIZE;
    inner_y = y % TILE_DOT_SIZE;

    int pi = pal_index(&ptn, inner_y, inner_x);
    if (pi) {
        //dprint("%d, %d, colind:%d\n", j, i, pi);
        set_data->l[inner_y].d[7 - inner_x] = plt.col[pi];
    }
    else {
        //transparent bg color is read from sprite 0x10 color.
        pi = vram_data_get(TRANSPARENT_PALETTE_ADDR);
        palette_index_to_rgb15(pi, &set_data->l[inner_y].d[7 - inner_x]);
    }

    return TRUE;
}


void set_sprite(int x, int y, int tile_id, struct sprite_attr sa) {
    struct palette plt;
    struct tile_2 ptn;
    int i, j;

    load_spr_attribute(sa, &plt);

    load_pattern(spr_pattern_bank, tile_id, &ptn);

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

int load_sprite_old(int foreground, int scanline) {
    int i;
    struct sprite_attr sa;
    unsigned char x, y, tile, tmp;

    //sprite priority:
    //draw lowest priority first, 
    //high priority late. highest priority comes top.
    for (i = SPRITE_CNT - 1; i >= 0; i--) {
        y = spr_ram_tbl_get(4 * i);
        if (scanline != y)
            continue;

        tmp = spr_ram_tbl_get(4 * i + 2);
        memcpy(&sa, &tmp, sizeof(struct sprite_attr));
        if (sa.priority != foreground)
            continue;

        tile = spr_ram_tbl_get(4 * i + 1);
        x = spr_ram_tbl_get(4 * i + 3);

        set_sprite(x, y, tile, sa);
        if (i == 0) {
            sprite0_hit_set();
        }
    }

    return TRUE;
}

//prefetch sprite step 1.
//load sprite temp buffer.
int sprite_prefetch1(int srch_line) {
    int i;
    unsigned char tmp;

    sprite_hit_cnt = 0;
    for (i = 0; i < SPRITE_CNT; i++) {
        int spr_y;
        spr_y = spr_ram_tbl_get(4 * i);
        if (srch_line < spr_y || srch_line > spr_y + TILE_DOT_SIZE)
            continue;

        if (sprite_hit_cnt < SPRITE_PREFETCH_CNT) {
            sprite_temp_buf[sprite_hit_cnt].y = spr_y;
            sprite_temp_buf[sprite_hit_cnt].index = spr_ram_tbl_get(4 * i + 1);
            tmp = spr_ram_tbl_get(4 * i + 2);
            memcpy(&sprite_temp_buf[sprite_hit_cnt].sa, &tmp, sizeof(struct sprite_attr));
            sprite_temp_buf[sprite_hit_cnt].x = spr_ram_tbl_get(4 * i + 3);
        }
/*
        dprint("sprite prefetch hit. #%d, index:%d, srch y:%d, spr y:%d, spr x:%d\n", 
            i, sprite_temp_buf[sprite_hit_cnt].index, srch_line, spr_y, 
            sprite_temp_buf[sprite_hit_cnt].x);
*/
        sprite_hit_cnt++;
    }
    return sprite_hit_cnt;
}

//prefetch sprite step 2.
//load sprite pattern and palette.
int sprite_prefetch2(int srch_line) {
    int i;
    int spr_buf_bottom;

    spr_buf_bottom = sprite_hit_cnt > SPRITE_PREFETCH_CNT ? SPRITE_PREFETCH_CNT : sprite_hit_cnt;
    for (i = 0; i < spr_buf_bottom; i++) {
        load_spr_attribute(sprite_temp_buf[i].sa, &sprite_buf[i].plt);
        load_pattern(spr_pattern_bank, sprite_temp_buf[i].index, &sprite_buf[i].ptn);
    }
    return 0;
}

int load_sprite(int foreground, int x, int y) {
    int i;
    int spr_buf_bottom;
    int pi;

    //sprite priority:
    //draw lowest priority first, 
    //high priority late. highest priority comes top.
    //seek for the topmost sprite on this spot (x, y)
    spr_buf_bottom = sprite_hit_cnt > SPRITE_PREFETCH_CNT ? SPRITE_PREFETCH_CNT : sprite_hit_cnt;
    for (i = 0; i < spr_buf_bottom; i++) {
        if (sprite_temp_buf[i].x <= x && x < sprite_temp_buf[i].x + TILE_DOT_SIZE) {
            int x_in, y_in;
            int draw_x_in, draw_y_in;

            if (sprite_temp_buf[i].sa.priority != foreground)
                continue;

            x_in = x % TILE_DOT_SIZE;
            y_in = y % TILE_DOT_SIZE;
            if (sprite_temp_buf[i].sa.flip_h) {
                draw_x_in = x_in;
                if (sprite_temp_buf[i].sa.flip_v) {
                    draw_y_in = 7 - y_in;
                }
                else {
                    draw_y_in = y_in;
                }
            }
            else {
                draw_x_in = 7 - x_in;
                if (sprite_temp_buf[i].sa.flip_v) {
                    draw_y_in = 7 - y_in;
                }
                else {
                    draw_y_in = y_in;
                }
            }

            pi = pal_index(&sprite_buf[i].ptn, draw_y_in, draw_x_in);
            if (pi) {
                //dprint("spr#%d, dot set x:%d, y:%d\n", sprite_temp_buf[i].index, x, y);
                vscreenn_dot_set(x, y, &sprite_buf[i].plt.col[pi]);
                if (sprite_temp_buf[i].index == 0) {
                    sprite0_hit_set();
                }
                return TRUE;
            }
        }
    }

    return FALSE;
}


void set_bg_pattern_bank(unsigned char bank) {
    bg_pattern_bank = bank;
}
void set_spr_pattern_bank(unsigned char bank) {
    spr_pattern_bank = bank;
}
void set_bg_name_tbl_base(unsigned char sw) {
    switch (sw) {
        case 0:
            bg_name_tbl_base = NAME0_START;
            break;
        case 1:
            bg_name_tbl_base = NAME1_START;
            break;
        case 2:
            bg_name_tbl_base = NAME2_START;
            break;
        case 3:
        default:
            bg_name_tbl_base = NAME3_START;
            break;
    }
    bg_attr_tbl_bank = sw;
}

struct rgb15 *get_vscreen_head(void) {
    return (struct rgb15 *)vscreen;
}

int vscreen_init(void) {
    bg_pattern_bank = 0;
    spr_pattern_bank = 0;
    bg_name_tbl_base = NAME0_START;
    bg_attr_tbl_bank = 0;

    sprite_hit_cnt = 0;

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

