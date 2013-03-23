#include <string.h>
#include <pthread.h>
#include <limits.h>
#include <stdio.h>

#include "tools.h"
#include "vram.h"
#include "ppucore.h"
#include "vga_xfer.h"

int vscreen_init(void);
void clean_vscreen(void);
int palette_init(void);
void vga_xfer(void);
void set_monocolor (int mono);
void set_nmi_pin(int val);
void set_bg_pattern_bank(unsigned char bank);
void set_spr_pattern_bank(unsigned char bank);
void set_bg_name_tbl_base(unsigned char sw);


void dump_ppu_reg(void);

/*
 * 6502 little endian
 * hi bit > low bit order
 * but gcc generates low > hi order for bit field
 * */
struct ppu_ctrl_reg1 {
    unsigned int name_tbl_sel   :2;
    unsigned int addr_inc_size  :1;
    unsigned int sprite_ptn_sel :1;
    unsigned int bg_ptn_addr_sel :1;
    unsigned int sprite_size    :1;
    unsigned int ppu_mode       :1;
    unsigned int nmi_vblank     :1;
} __attribute__ ((packed));

struct ppu_ctrl_reg2 {
    unsigned int color_mode     :1;
    unsigned int show_left_8bg  :1;
    unsigned int show_left_8sprite  :1;
    unsigned int show_bg        :1;
    unsigned int show_sprite    :1;
    unsigned int intense_b      :1;
    unsigned int intense_g      :1;
    unsigned int intense_r      :1;
} __attribute__ ((packed));

/*
 * 6502 little endian
 * but gcc generates low > hi order when bit field is used??
 * */
struct ppu_status_reg {
    unsigned int nouse          :4;
    unsigned int vram_ignore    :1;
    unsigned int sprite_overflow    :1;
    unsigned int sprite_0_hit   :1;
    unsigned int vblank         :1;
} __attribute__ ((packed));

struct ppu_vram_addr_reg {
    union {
        struct {
            unsigned char low;
            unsigned char hi;
        } b;
        unsigned short s;
    } addr;
    unsigned int cnt :1;
};

/*ppu core register instances*/

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

//value set by the ctrl_reg1.
static unsigned char    sprite_size_type;
static unsigned char    vram_addr_inc;
static unsigned int     vram_read_cnt;

#define SPR_STYPE_8x8   0 
#define SPR_STYPE_8x16  1

static pthread_t ppucore_thread_id;
static int ppucore_end_loop;

/*
 * ppucore main loop.
 * periodically update the display buffer.
 * */
static void *ppucore_loop(void* arg) {
    //struct timespec ts = {CPU_CLOCK_SEC, CPU_CLOCK_NSEC / 10};
    struct timespec begin, end;
    struct timespec slp;
    long sec, nsec;
#define NANOMAX (1000000000 - 1)

    while (!ppucore_end_loop) {
        int updated = FALSE;

        //start displaying
        status_reg.vblank = 0;
        status_reg.vram_ignore = 1;

        clock_gettime(CLOCK_REALTIME, &begin);
        if (ctrl_reg2.show_sprite) {
            //sprite in the back
            ;
        }
        if (ctrl_reg2.show_bg/**/) {
            //back ground image
            updated |= show_background();
        }
        if (ctrl_reg2.show_sprite) {
            //foreground sprite
            ;
        }
        if (updated) 
            vga_xfer();

        //printing display done.
        status_reg.vblank = 1;
        status_reg.vram_ignore = 0;
        if (ctrl_reg1.nmi_vblank) {
            //generate nmi interrupt to the cpu.
            set_nmi_pin(TRUE);
        }

        clock_gettime(CLOCK_REALTIME, &end);

        //sleep rest of time...
        if (end.tv_sec < begin.tv_sec )
            sec = LONG_MAX - begin.tv_sec + end.tv_sec + 1;
        else
            sec = end.tv_sec - begin.tv_sec;

        if (end.tv_nsec < begin.tv_nsec)
            nsec = NANOMAX - begin.tv_nsec + end.tv_nsec + 1;
        else
            nsec = end.tv_nsec - begin.tv_nsec;

        if (sec < NES_VIDEO_CLK_SEC || nsec < NES_VIDEO_CLK_NSEC) {
            int ret;
            slp.tv_sec = sec > NES_VIDEO_CLK_SEC ? 0 : NES_VIDEO_CLK_SEC - sec;
            slp.tv_nsec = nsec > NES_VIDEO_CLK_NSEC ? 0 : NES_VIDEO_CLK_NSEC - nsec;

            //dprint("%d.%09d sec sleep\n", slp.tv_sec, slp.tv_nsec);
            ret = nanosleep(&slp, NULL);
        }
    }
    return NULL;
}

void ppu_ctrl1_set(unsigned char data) {
    unsigned char old, diff_tmp; 
    struct ppu_ctrl_reg1 diff; 

    memcpy(&old, &ctrl_reg1, sizeof(ctrl_reg1));
    memcpy(&ctrl_reg1, &data, sizeof(ctrl_reg1));

    diff_tmp = old ^ data;
    memcpy(&diff, &diff_tmp, sizeof(ctrl_reg1));

    //set sprite_size
    //if (diff.sprite_size)
    //    sprite_size = (ctrl_reg1.sprite_size == 0 ? 8 : 16);

    //set bg base tbl addr
    if (diff.bg_ptn_addr_sel)
        set_bg_pattern_bank(ctrl_reg1.bg_ptn_addr_sel);
    //set sprite base tbl addr
    if (diff.sprite_ptn_sel) 
        set_spr_pattern_bank(ctrl_reg1.sprite_ptn_sel);
    //set vram address increase unit
    if (diff.addr_inc_size) 
        vram_addr_inc = (ctrl_reg1.addr_inc_size == 0 ? 1 : 32);
    //set main screen addr
    if (diff.name_tbl_sel)
        set_bg_name_tbl_base(ctrl_reg1.name_tbl_sel);

    //dprint("ctrl1: %x\n", data);
    //dump_ppu_reg();
}

void ppu_ctrl2_set(unsigned char data) {
    struct ppu_ctrl_reg2 old = ctrl_reg2;

    memcpy(&ctrl_reg2, &data, sizeof(ctrl_reg2));
    //dprint("ppu_ctrl2_set %d, show_bg:%d\n", data, ctrl_reg2.show_bg);

    if (old.color_mode != ctrl_reg2.color_mode)
        set_monocolor(ctrl_reg2.color_mode);

    //dprint("ctrl2 %x:\n", data);
    //dump_ppu_reg();
}

unsigned char ppu_status_get(void) {
    unsigned char ret;
    memcpy(&ret, &status_reg, sizeof(status_reg));

    //if read status reg, vram addr register counter is reset
    vram_addr_reg.cnt = 0;
    //dprint("ppu_status:%x\n", ret);
    return ret;
}

void sprite_addr_set(unsigned char addr) {
    sprite_ram_addr_reg = addr;
}

void sprite_data_set(unsigned char data) {
    sprite_ram_data_reg = data;
}

void ppu_scroll_set(unsigned char data) {
    scroll_reg = data;
}

void ppu_vram_addr_set(unsigned char half_addr) {
    //dprint("vram addr:%04x\n", half_addr);
    if (vram_addr_reg.cnt++ == 0)
        vram_addr_reg.addr.b.hi = half_addr;
    else
        vram_addr_reg.addr.b.low = half_addr;

    //when setting the addr, read cnt is reset.
    vram_read_cnt = 0;
}

void ppu_vram_data_set(unsigned char data) {
    //check vram_ignore bit on write.
    /*
    */
    if (status_reg.vram_ignore)
        return;

    //dprint("vram data:%04x\n", data);
    vram_data_reg = data;

    vram_data_set(vram_addr_reg.addr.s, data);
    //vram addr increment.
    vram_addr_reg.addr.s += vram_addr_inc;
}

unsigned char ppu_vram_data_get(void) {
    if (vram_read_cnt++ == 0) {
        //first read always fail.
        return 0;
    }

    vram_data_reg = vram_data_get(vram_addr_reg.addr.s);
    //vram addr increment.
    vram_addr_reg.addr.s += vram_addr_inc;
    return vram_data_reg;
}

int ppucore_init(void) {
    int ret;
    pthread_attr_t attr;

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
    vram_read_cnt = 0;

    sprite_size_type = SPR_STYPE_8x8;
    vram_addr_inc = 1;

    ret = vga_xfer_init();
    if (!ret)
        return FALSE;

    ret = vscreen_init();
    if (!ret)
        return FALSE;

    ret = palette_init();
    if (!ret)
        return FALSE;

    ret = vram_init();
    if (!ret)
        return FALSE;

    ppucore_end_loop = FALSE;
    ret = pthread_attr_init(&attr);
    if (ret != RT_OK) {
        return FALSE;
    }
    ppucore_thread_id = 0;
    ret = pthread_create(&ppucore_thread_id, &attr, ppucore_loop, NULL);
    if (ret != RT_OK) {
        return FALSE;
    }


    return TRUE;
}

void clean_ppucore(void) {
    void* ret;
    ppucore_end_loop = TRUE;
    pthread_join(ppucore_thread_id, &ret);
    dprint("ppucore thread joined.\n");

    clean_vram();
    clean_vscreen();
}

/*
 * for debug.c
 * */
void dump_ppu_reg(void) {
    printf("control reg1\n");
    printf(" nmi_vblank:%d\n", ctrl_reg1.nmi_vblank);
    printf(" sprite_size:%d\n", ctrl_reg1.sprite_size);
    printf(" bg_ptn:%d\n", ctrl_reg1.bg_ptn_addr_sel);
    printf(" spr_ptn:%d\n", ctrl_reg1.sprite_ptn_sel);
    printf(" inc size:%d\n", ctrl_reg1.addr_inc_size);
    printf(" name tbl:%d\n", ctrl_reg1.name_tbl_sel);

    printf("\ncontrol reg2\n");
    printf(" intense r:%d\n", ctrl_reg2.intense_r);
    printf(" intense g:%d\n", ctrl_reg2.intense_g);
    printf(" intense b:%d\n", ctrl_reg2.intense_b);

    printf(" show spr:%d\n", ctrl_reg2.show_sprite);
    printf(" show bg:%d\n", ctrl_reg2.show_bg);
    printf(" left 8 pix spr:%d\n", ctrl_reg2.show_left_8sprite);
    printf(" left 8 pix bg:%d\n", ctrl_reg2.show_left_8bg);
    printf(" col mode:%d\n", ctrl_reg2.color_mode);

}

