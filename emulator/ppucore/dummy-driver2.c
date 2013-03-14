#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "tools.h"
#include "vga.h"
#include "vram.h"
#include "sprite.h"

static struct rgb15 *disp_data;
void *vga_shm_get(void);
void vga_shm_free(void* addr);
int ppucore_init(void);
void clean_ppucore(void);
int load_cartridge(const char* cartridge);
void set_vga_base(unsigned char* base);
void vga_xfer(void);
void dump_vram(int type, int bank, unsigned short addr, int size);
void set_bgtile(int tile_id);
void set_sprite(int x, int y, int tile_id, struct sprite_attr sa);
void set_monocolor (int mono);

struct timespec sleep_inteval = {0, 1000000 / VGA_REFRESH_RATE};


/*
 * dummy function
 * for cartridge.c
 * */
int load_prg_rom(FILE* cartridge, int num_rom_bank) {
#define ROM_32K 0x8000
    int len;
    char *rom_buffer;

    rom_buffer = malloc(ROM_32K);
    if (rom_buffer == NULL)
        return FALSE;
    len = fread(rom_buffer, 1, ROM_32K, cartridge);
    if (len != ROM_32K)
        return FALSE;

    free(rom_buffer);
    return TRUE;
}

/*
 * for debug.c
 * */
int debug_mode = TRUE;
int critical_error = FALSE;
void dump_6502(int full) {
}
unsigned char dbg_get_byte(unsigned short addr) {
        return 0;
}
unsigned short dbg_get_short(unsigned short addr) {
        return 0;
}

/*
 * ppu test function
 * */
static void test_ppu(void) {
    int i;
    unsigned char plt[32] = {
            0x0f, 0x00, 0x10, 0x20,
            0x0f, 0x06, 0x16, 0x26,
            0x0f, 0x08, 0x18, 0x28,
            0x0f, 0x0a, 0x1a, 0x2a,

            0x0f, 0x00, 0x10, 0x20,
            0x0f, 0x06, 0x16, 0x26,
            0x0f, 0x08, 0x18, 0x28,
            0x0f, 0x0a, 0x1a, 0x2a,
/*
        0, 5, 1,  0x28,  0, 6,  0xb,  0x36, 
        0, 9, 10, 11, 0, 13, 14, 15,
        0, 30, 31, 32, 0, 40, 41, 42,
        0, 20, 21, 22, 0, 11, 12, 13
*/
    };

    for (i = 0; i < 16; i++)
        bg_palette_tbl_set(i, plt[i]);
    for (i = 0; i < 16; i++)
        spr_palette_tbl_set(i, plt[i + 16]);

    for (i = 0; i < 960; i++) 
        name_tbl_set(0, i, 0);

    for (i = 0; i < 64; i++) 
        attr_tbl_set(0, i, 0);

    //name_tbl_set(0, 205, 2);
    name_tbl_set(0, 205, 'D');
    name_tbl_set(0, 206, 'e');
    name_tbl_set(0, 207, 'e');
    name_tbl_set(0, 208, '!');
    name_tbl_set(0, 209, '!');
    //205 = palette gp2 00011011b 
    //205 = 11
    attr_tbl_set(0, 11, 0x1b);

    //other test.
    name_tbl_set(0, 300, 1);
    attr_tbl_set(0, 0, 0x65);

    set_monocolor(FALSE);

    for (i = 0; i < 960; i++) 
        set_bgtile(i);

    //sprite test
    struct sprite_attr sa;
    sa.palette = 2;
    sa.priority = 1;
    sa.flip_h = 0;
    sa.flip_v = 0;
    set_sprite(30, 100, 'd', sa);
    sa.flip_h = 1;
    set_sprite(50, 100, 'd', sa);
    sa.flip_v = 1;
    set_sprite(70, 105, 'd', sa);

    struct timespec begin, end;
    clock_gettime(CLOCK_REALTIME, &begin);
    vga_xfer();
    clock_gettime(CLOCK_REALTIME, &end);

    dprint("vga_xfer elapsed time: %d.%09d, vga frame rate:0.%09d\n", 
            end.tv_sec - begin.tv_sec, 
            end.tv_nsec - begin.tv_nsec, 
            1000000000 / 60);
    fflush(stdout);

//void dump_vram(int type, int bank, unsigned short addr, int size);
/*
    dump_vram(VRAM_DUMP_TYPE_PTN, 0, 0, 0x100);
    dump_vram(VRAM_DUMP_TYPE_NAME, 0, 0, 300);
    dump_vram(VRAM_DUMP_TYPE_ATTR, 0, 0, 64);
    dump_vram(VRAM_DUMP_TYPE_PLT, 0, 0, 16);
*/
}

int main(int argc, char** argv) {
    int ret;

    ret = ppucore_init();
    if (ret == FALSE) {
        fprintf(stderr, "ppucore init error.\n");
        return -1;
    }
    ret = load_cartridge("sample1.nes");
    if (ret == FALSE) {
        fprintf(stderr, "load cartridge error.\n");
        return -1;
    }

    /* get vga shared memory */
    if((disp_data = (struct rgb15 *)vga_shm_get()) == NULL)
    {
        fprintf(stderr, "error attaching shared memory.\n");
        return -1;
    }

    memset(disp_data, 0, VGA_SHM_SIZE);
    set_vga_base((unsigned char*)disp_data);


    /////test...
    test_ppu();

    clean_ppucore();
    vga_shm_free(disp_data);
    
    return 0;
}


