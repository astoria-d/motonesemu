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

static struct rgb15 *disp_data;
void *vga_shm_get(void);
void vga_shm_free(void* addr);
int ppucore_init(void);
void clean_ppucore(void);
int load_cartridge(const char* cartridge);
void set_vga_base(unsigned char* base);
void vga_xfer(void);

struct timespec sleep_inteval = {0, 1000000 / VGA_REFRESH_RATE};


/*dummy function*/
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

static void test_ppu(void) {
    int i;
    unsigned char plt[32] = {
        0x0f, 0x00, 0x10, 0x20,
        0x0f, 0x06, 0x16, 0x26,
        0x0f, 0x08, 0x18, 0x28,
        0x0f, 0x0a, 0x1a, 0x2a
    };

    for (i = 0; i < 16; i++)
        img_palette_tbl_set(i, plt[i]);
    for (i = 0; i < 16; i++)
        spr_palette_tbl_set(i, plt[i + 16]);
    attr_tbl_set(0, 100, 0);
    name_tbl_set(0, 100, 'm');
    vga_xfer();
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


