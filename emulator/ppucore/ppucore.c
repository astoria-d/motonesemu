#include <string.h>
#include <pthread.h>
#include <limits.h>

#include "tools.h"
#include "vram.h"
#include "ppucore.h"
#include "vga_xfer.h"

int vscreen_init(void);
void clean_vscreen(void);
int palette_init(void);
void vga_xfer(void);

/*
 * 6502 little endian
 * hi bit > low bit order
 * */
struct ppu_ctrl_reg1 {
    unsigned int nmi_vblank     :1;
    unsigned int ppu_mode       :1;
    unsigned int sprite_size    :1;
    unsigned int bg_ptn_addr_sel :1;
    unsigned int sprite_ptn_sel :1;
    unsigned int addr_inc_size  :1;
    unsigned int name_tbl_sel   :2;
} __attribute__ ((packed));

struct ppu_ctrl_reg2 {
    unsigned int intense_b      :1;
    unsigned int intense_g      :1;
    unsigned int intense_r      :1;
    unsigned int show_sprite    :1;
    unsigned int show_bg        :1;
    unsigned int show_left_8sprite  :1;
    unsigned int show_left_8bg  :1;
    unsigned int color_mode     :1;
} __attribute__ ((packed));

struct ppu_status_reg {
    unsigned int vblank         :1;
    unsigned int sprite_0_hit   :1;
    unsigned int sprite_overflow    :1;
    unsigned int vram_ignore    :1;
    unsigned int nouse          :4;
} __attribute__ ((packed));

struct ppu_vram_addr_reg {
    unsigned int cnt :1;
    unsigned char addr1;
    unsigned char addr2;
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


static pthread_t ppucore_thread_id;
static int ppucore_end_loop;

static void *ppucore_loop(void* arg) {
    //struct timespec ts = {CPU_CLOCK_SEC, CPU_CLOCK_NSEC / 10};
    struct timespec begin, end;
    struct timespec slp;
    long sec, nsec;
#define NANOMAX (1000000000 - 1)

    while (!ppucore_end_loop) {

        clock_gettime(CLOCK_REALTIME, &begin);
        if (ctrl_reg2.show_sprite) {
            //sprite in the back
            ;
        }
        if (ctrl_reg2.show_bg) {
            //back ground image
            show_background();
        }
        if (ctrl_reg2.show_sprite) {
            //foreground sprite
            ;
        }
        vga_xfer();
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

        //dprint("sec:%d, nsec:%d\n", sec, nsec);
        if (sec < NES_VIDEO_CLK_SEC || nsec < NES_VIDEO_CLK_NSEC) {
            int ret;
            slp.tv_sec = sec > NES_VIDEO_CLK_SEC ? 0 : NES_VIDEO_CLK_SEC - sec;
            slp.tv_nsec = nsec > NES_VIDEO_CLK_NSEC ? 0 : NES_VIDEO_CLK_NSEC - nsec;

            ret = nanosleep(&slp, NULL);
            //dprint("sleep %d sec:%d, nsec:%d, err:%d\n", ret, ts.tv_sec, ts.tv_nsec, errno);
        }
    }
    return NULL;
}

void ppu_ctrl1_set(unsigned char data) {
    memcpy(&ctrl_reg1, &data, sizeof(ctrl_reg1));
}

void ppu_ctrl2_set(unsigned char data) {
    memcpy(&ctrl_reg2, &data, sizeof(ctrl_reg2));
    //dprint("ppu_ctrl2_set %d, show_bg:%d\n", data, ctrl_reg2.show_bg);
}

unsigned char ppu_status_get(void) {
    unsigned char ret;
    memcpy(&ret, &status_reg, sizeof(status_reg));
    return ret;
}

void sprite_addr_set(unsigned char data) {
    sprite_ram_addr_reg = data;
}

void sprite_data_set(unsigned char data) {
    sprite_ram_data_reg = data;
}

void ppu_scroll_set(unsigned char data) {
    scroll_reg = data;
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

    ret = vram_init();
    if (!ret)
        return FALSE;

    ret = vga_xfer_init();
    if (!ret)
        return FALSE;

    ret = vscreen_init();
    if (!ret)
        return FALSE;

    ret = palette_init();
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
