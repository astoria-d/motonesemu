#include <string.h>

#include "tools.h"

struct ppu_ctrl_reg1 {
    unsigned int name_tbl_sel   :2;
    unsigned int addr_inc_size  :1;
    unsigned int sprite_ptn_sel :1;
    unsigned int bg_ptn_addr_sel :1;
    unsigned int sprite_size    :1;
    unsigned int ppu_mode       :1;
    unsigned int nmi_vblank     :1;
};

struct ppu_ctrl_reg2 {
    unsigned int color_mode     :1;
    unsigned int show_left_8bg  :1;
    unsigned int show_left_8sprite  :1;
    unsigned int show_bg        :1;
    unsigned int show_sprite    :1;
    unsigned int intense_r      :1;
    unsigned int intense_g      :1;
    unsigned int intense_b      :1;
};

struct ppu_status_reg {
    unsigned int dummy          :4;
    unsigned int vram_ignore    :1;
    unsigned int sprite_overflow    :1;
    unsigned int sprite_0_hit   :1;
    unsigned int vblank         :1;
};

struct ppu_vram_addr_reg {
    unsigned int cnt :1;
    unsigned char addr1;
    unsigned char addr2;
};

static struct ppu_ctrl_reg1 ctrl_reg1;
static struct ppu_ctrl_reg2 ctrl_reg2;
static struct ppu_status_reg status_reg;
static struct ppu_vram_addr_reg vram_addr_reg;

static unsigned char sprite_ram_addr_reg;
static unsigned char sprite_ram_data_reg;
static unsigned char sprite_ram_dma_reg;
static unsigned char vram_data_reg;
static unsigned char scroll_reg;
static unsigned char vram_dma_reg;

void ppu_ctrl1_set(unsigned char data) {
    ctrl_reg1 = *(struct ppu_ctrl_reg1*)&data;
}

void ppu_ctrl2_set(unsigned char data) {
    ctrl_reg2 = *(struct ppu_ctrl_reg2*)&data;
}

unsigned char ppu_status_get(void) {
    return *(unsigned char*)&status_reg;
}

void sprite_addr_set(unsigned char data) {
    sprite_ram_addr_reg = data;
}

unsigned char sprite_data_get(void) {
    return sprite_ram_data_reg;
}

void ppu_vram_addr_set(unsigned char data) {
    if (vram_addr_reg.cnt++ == 0)
        vram_addr_reg.addr1 = data;
    else
        vram_addr_reg.addr2 = data;
}

void ppu_vram_data_set(unsigned char data) {
    vram_data_reg = data;
}

unsigned char ppu_vram_data_get(void) {
    return vram_data_reg;
}

int ppucore_init(void) {

    memset(&ctrl_reg1, 0, sizeof(ctrl_reg1));
    memset(&ctrl_reg2, 0,  sizeof(ctrl_reg1));
    memset(&status_reg, 0, sizeof(status_reg));
    memset(&vram_addr_reg, 0, sizeof(vram_addr_reg));

    sprite_ram_addr_reg = 0;
    sprite_ram_data_reg = 0;
    sprite_ram_dma_reg = 0;
    vram_data_reg = 0;
    scroll_reg = 0;
    vram_dma_reg = 0;

    return TRUE;
}

