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
void load_spr_palette(struct sprite_attr sa, struct palette *plt);
void sprite0_hit_set(void);
void sprite_overflow_set(void);
unsigned char spr_ram_tbl_get(unsigned short offset);
unsigned char vram_data_get(unsigned short addr);
void palette_index_to_rgb15(unsigned char index, struct rgb15* rgb);

#define TRANSPARENT_PALETTE_ADDR        0x3F10

struct sprite_buf_reg {
    struct palette plt;
    struct tile_2 ptn;
    int sprite_num;
};

static struct rgb15 *vscreen;

static unsigned char bg_pattern_bank;
static unsigned char spr_pattern_bank;
static unsigned short   bg_name_tbl_base;
static unsigned char    bg_attr_tbl_bank;

#define SPRITE_PREFETCH_CNT     8
static struct ppu_sprite_reg sprite_temp_buf [SPRITE_PREFETCH_CNT];
static struct sprite_buf_reg sprite_buf [SPRITE_PREFETCH_CNT];
static int sprite_hit_cnt;
static int bg_transparent;
static int bg_sprite;

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

static struct palette bg_plt;
static struct tile_2 bg_ptn;
static struct rgb15* set_data;

struct rgb15* get_current_vscreen(void) {
    return set_data;
}

void set_vscreen_pos(int x, int y) {
    if (x == 0 && y == 0) {
        set_data = vscreen;
    }
    else {
        set_data++;
    }
}

int load_background(int x, int y) {
    //dprint("load bg x:%d, y:%d...\n", x, y);
    int inner_x, inner_y;
    unsigned short name_base;
    unsigned short attr_bank;

    //TODO name/attr table switch must be set with respect to the cartridge mapper setting.
    if (x >= VSCREEN_WIDTH) {
        x -= VSCREEN_WIDTH;
        if (y >= VSCREEN_HEIGHT) {
            y -= VSCREEN_HEIGHT;
            name_base = bg_name_tbl_base + (NAME_TBL_SIZE + ATTR_TBL_SIZE) * 3;
            attr_bank = bg_attr_tbl_bank + 3;
        }
        else {
            name_base = bg_name_tbl_base + NAME_TBL_SIZE + ATTR_TBL_SIZE;
            attr_bank = bg_attr_tbl_bank + 1;
        }
    }
    else {
        if (y >= VSCREEN_HEIGHT) {
            y -= VSCREEN_HEIGHT;
            name_base = bg_name_tbl_base + (NAME_TBL_SIZE + ATTR_TBL_SIZE) * 2;
            attr_bank = bg_attr_tbl_bank + 2;
        }
        else {
            name_base = bg_name_tbl_base;
            attr_bank = bg_attr_tbl_bank;
        }
    }

    //tile loading happens every 8 dots only.
    //TODO must check if the tile is loaded due to the in-draw scrolling??.
    if (x % TILE_DOT_SIZE == 0) {
        int tile_id, tile_id_x, tile_id_y;
        unsigned char name_index;

        tile_id_x = x / TILE_DOT_SIZE;
        tile_id_y = y / TILE_DOT_SIZE;
        tile_id = tile_id_x + tile_id_y * H_SCREEN_TILE_SIZE;

        //dprint("load tile.\n");
        load_attribute(attr_bank, tile_id, &bg_plt);
        name_index = vram_data_get(name_base + tile_id);
        load_pattern(bg_pattern_bank, name_index, &bg_ptn);
    }

    //pattern dot is stored right to left order (little endian.).
    inner_x = 7 - x % TILE_DOT_SIZE;
    inner_y = y % TILE_DOT_SIZE;

    int pi = pal_index(&bg_ptn, inner_y, inner_x);
    if (pi) {
        //dprint("%d, %d, colind:%d\n", j, i, pi);
        *set_data = bg_plt.col[pi];
        bg_transparent = FALSE;
    }
    else if (!bg_sprite){
        //transparent bg color is read from sprite 0x10 color.
        pi = vram_data_get(TRANSPARENT_PALETTE_ADDR);
        palette_index_to_rgb15(pi, set_data);
        bg_transparent = TRUE;
    }

    return TRUE;
}

//prefetch sprite step 1.
//load sprite temp buffer.
int sprite_prefetch1(int srch_line) {
    int i;
    unsigned char tmp;

    sprite_hit_cnt = 0;
    for (i = SPRITE_CNT - 1; i >= 0; i--) {
        int spr_y;
        spr_y = spr_ram_tbl_get(4 * i);
        if (spr_y <= srch_line && srch_line < spr_y + TILE_DOT_SIZE) {
            if (sprite_hit_cnt < SPRITE_PREFETCH_CNT) {
                sprite_temp_buf[sprite_hit_cnt].y = spr_y;
                sprite_temp_buf[sprite_hit_cnt].index = spr_ram_tbl_get(4 * i + 1);
                tmp = spr_ram_tbl_get(4 * i + 2);
                memcpy(&sprite_temp_buf[sprite_hit_cnt].sa, &tmp, sizeof(struct sprite_attr));
                sprite_temp_buf[sprite_hit_cnt].x = spr_ram_tbl_get(4 * i + 3);
                sprite_buf[sprite_hit_cnt].sprite_num = i;
            }
            /*
               dprint("sprite prefetch hit. #%d, index:%d, srch y:%d, spr y:%d, spr x:%d\n", 
               i, sprite_temp_buf[sprite_hit_cnt].index, srch_line, spr_y, 
               sprite_temp_buf[sprite_hit_cnt].x);
               */
            sprite_hit_cnt++;
        }
    }
    if (sprite_hit_cnt > SPRITE_PREFETCH_CNT) {
        sprite_overflow_set();
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
        load_spr_palette(sprite_temp_buf[i].sa, &sprite_buf[i].plt);
        load_pattern(spr_pattern_bank, sprite_temp_buf[i].index, &sprite_buf[i].ptn);
    }
    return spr_buf_bottom;
}

int load_sprite(int background, int x, int y) {
    int i;
    int spr_buf_bottom;
    int pi;

    if (background)
        bg_sprite = FALSE;

    //sprite priority:
    //draw lowest priority first, 
    //high priority late. highest priority comes top.
    //seek for the topmost sprite on this spot (x, y)
    spr_buf_bottom = sprite_hit_cnt > SPRITE_PREFETCH_CNT ? SPRITE_PREFETCH_CNT : sprite_hit_cnt;
    for (i = spr_buf_bottom - 1; i >= 0; i--) {
        if (sprite_temp_buf[i].x <= x && x < sprite_temp_buf[i].x + TILE_DOT_SIZE) {
            int x_in, y_in;
            int draw_x_in, draw_y_in;

            //set_data->r = set_data->g = set_data->b = 0;

            if (sprite_temp_buf[i].sa.priority != background)
                continue;

            x_in = x - sprite_temp_buf[i].x;
            y_in = y - sprite_temp_buf[i].y;
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
                //dprint("spr#%d, id:%d, dot set x:%d, y:%d\n", 
                 //       sprite_buf[i].sprite_num, sprite_temp_buf[i].index, x, y);
                *set_data = sprite_buf[i].plt.col[pi];
                if (sprite_temp_buf[i].sa.priority && sprite_buf[i].sprite_num == 0)
                    sprite0_hit_set();
                if (background)
                    bg_sprite = TRUE;
                return TRUE;
            }
        }
    }
    //spr_data->r = spr_data->g = spr_data->b = 0;

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

int vscreen_init(void) {
    bg_pattern_bank = 0;
    spr_pattern_bank = 0;
    bg_name_tbl_base = NAME0_START;
    bg_attr_tbl_bank = 0;

    sprite_hit_cnt = 0;

    vscreen = (struct rgb15 *) malloc(
        sizeof (struct rgb15) * H_SCREEN_TILE_SIZE * V_SCREEN_TILE_SIZE 
        * TILE_DOT_SIZE * TILE_DOT_SIZE);
    if (vscreen == NULL)
        return FALSE;
    memset(vscreen, 0, sizeof (struct rgb15) * H_SCREEN_TILE_SIZE * V_SCREEN_TILE_SIZE 
        * TILE_DOT_SIZE * TILE_DOT_SIZE);
    set_data = NULL;

    //dprint("tile_1_line:%d tile_2 size:%d\n", sizeof(struct tile_1_line), sizeof(struct tile_2));

    return TRUE;
}

void clean_vscreen(void) {
    free(vscreen);
}

