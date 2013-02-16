#include <stdio.h>
#include "tools.h"

struct nes_cart_header {
    unsigned char magic[4];
    unsigned char num_prom_banks;
    unsigned char num_crom_banks;
    unsigned char rom_ctrl1;
    unsigned char rom_ctrl2;
    unsigned char num_ram_banks;
    unsigned char researved[7];
};

static unsigned char num_prom_banks;
static unsigned char num_crom_banks;
static unsigned char num_ram_banks;
static unsigned char has_trainer;

#define ROM_VERTICAL_MIRROR(X)      (0x1 & X == 0x1)
#define ROM_HORIZONTAL_MIRROR(X)    (0x1 & X == 0x0)
#define BATTERY_BACKUP(X)           ((0x2 & X) > 0)
#define TRAINER(X)                  ((0x4 & X) > 0)
#define FOUR_SCRN_MIRROR(X)         ((0x8 & X) > 0)

int load_rom_file(FILE* cartridge, int num_rom_bank);

int check_cart_file(FILE* fp) {
    struct nes_cart_header cf;
    int len;
    int ret;

    len = fread(&cf, 1, sizeof(struct nes_cart_header), fp);
    if (len != sizeof(struct nes_cart_header))
        return FALSE;
    if(cf.magic[0] != 'N' ||
        cf.magic[1] != 'E' ||
        cf.magic[2] != 'S' ||
        cf.magic[3] != 0x1A) {
        return FALSE;
    }
    num_prom_banks = cf.num_prom_banks;
    num_crom_banks = cf.num_crom_banks;
    num_ram_banks = cf.num_ram_banks;
    has_trainer = TRAINER(cf.rom_ctrl1);

    if (has_trainer) {
        dprint("skip trainer...\n");
        ret = fseek(fp, 512, SEEK_CUR);
        if (ret) {
            fclose(fp);
            return FALSE;
        }
    }

    ret = load_rom_file(fp, num_prom_banks);
    if (!ret) {
        fprintf(stderr, "rom load err.\n");
        fclose(fp);
        return FALSE;
    }
    
    return TRUE;
}


int load_cartridge(const char* cartridge) {
    FILE* f_cart;
    int ret;

    f_cart = fopen(cartridge, "r");
    if (f_cart == NULL)
        return FALSE;

    ret = check_cart_file(f_cart);
    if (!ret) {
        fclose(f_cart);
        return FALSE;
    }

    fclose(f_cart);
    return TRUE;
}



