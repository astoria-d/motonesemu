#include <stdio.h>
#include <string.h>
#include <curses.h>
#include "tools.h"
#include "6502core.h"
#include "mapper.h"

#define MAXBUF  1024
#define BYTES_PER_LINE 16

extern int debug_mode;
void dump_6502(int full);
void dump_vram(unsigned short addr, int size);
void dump_sprite_ram(unsigned short addr, int size);
void dump_mem(unsigned short addr, int size);
static void dump_mem_to_file(unsigned short addr, int size, const char* fname);
unsigned char vram_data_get(unsigned short addr);
unsigned char spr_ram_tbl_get(unsigned short offset);
unsigned char dbg_get_byte(unsigned short addr);
unsigned short dbg_get_short(unsigned short addr);
void dump_ppu_reg(void);
void d1_set(int on_off);
void d2_set(int on_off);
void d3_set(int on_off);
void d4_set(int on_off);
void d6_set(int on_off);
void reset_cpu(void);
unsigned long get_clock_cnt(void);
unsigned int get_nmi_cnt(void);

#define MAX_HISTORY     10

struct cmd_list {
    struct dlist l;
    char *cmd;
};

static struct cmd_list* debug_history;
static int d5_disas;

//global variable.
unsigned short break_point;
unsigned long long break_counter_point;
unsigned short break_nmi_point;

static int d2_short;

mp_set_debugger_t mp_set_debugger;
unsigned char dbg_rom_get_byte(unsigned short offset);
unsigned short dbg_rom_get_short(unsigned short offset);

static void print_debug(void) {
    printf("   command:\n");
    printf("            s: step\n");
    printf("            c: continue\n");
    printf("            r: reset\n");
    printf("       b addr: set break point\n");
    printf("               (break point can be set only 1 address.)\n");
    printf("       bc counter: set break point at specified cpu counter\n");
    printf("               (counter is set in 64 bit long hex form.)\n");
    printf("          del: delete break point\n");
    printf("  m addr size: memory dump\n");
    printf("  mf addr size fname: binary memory dump to file\n");
    printf("         show: show registers\n");
    printf("        pshow: show ppu registers\n");
    printf("  v addr size: vram dump\n");
    printf("  spr addr size: sprite dump\n");
    printf("  da addr size: disassemble\n");
    printf("   d1 on/off: debug log level 1 (dump instruction on execution)\n");
    printf("   d2 0/1/2:  debug log level 2 (dump reg status for each instruction. 0=off, 1=full, 2=short.\n                                 dumped registers order is acc, x, y, sp, status.)\n");
    printf("   d3 on/off: debug log level 3 (dump load/store data value)\n");
    printf("   d4 on/off: debug log level 4 (dump vram data write)\n");
    printf("   d5 on/off: debug log level 5 (dump clock counter w/ d1)\n");
    printf("   d6 on/off: debug log level 6 (report sprite hit position)\n");
    printf("            q: quit emulator\n");
}

#if 0
static int read_cmd(char* buf) {
    char ch, *p;
    p = buf;
            
    while(1) {
        if (kbhit()) {
            ch = getchar();
            if ( ch  != '\n') {
                *p = ch;
                p++;
                printf("%c", ch);
            }
            else {
                printf("\n");
                break;
            }
        }
    }
    return strlen(buf);
}
#endif

int emu_debug(void) {
    char buf[MAXBUF];

    //pause_cpu_clock();
    while (TRUE) {
        /*
        fflush(stdin);
        fflush(stdout);
        */
        printf("motonesemu: ");
        memset(buf, 0, sizeof(buf));
        //read_cmd(buf);
        scanf("%s", buf);

        if (!strcmp(buf, "s")){
            break;
        }
        else if (!strcmp(buf, "q")){
            extern int critical_error;

            printf("quit...\n");
            debug_mode = FALSE;
            critical_error = TRUE;
            //raise(SIGINT);
            return FALSE;
        }
        else if (!strcmp(buf, "c")){
            debug_mode = FALSE;
            break;
        }
        else if (!strcmp(buf, "r")){
            reset_cpu();
        }
        else if (!strcmp(buf, "show")){
            dump_6502(TRUE);
        }
        else if (!strcmp(buf, "d1")){
            scanf("%s", buf);
            if (!strcmp(buf, "on")){
                d1_set(TRUE);
            }
            else if (!strcmp(buf, "off")){
                d1_set(FALSE);
            }
            else {
                printf("log parameter must be either [on] or [off].\n");
            }
        }
        else if (!strcmp(buf, "d2")){
            scanf("%s", buf);
            if (!strcmp(buf, "1")){
                d2_set(TRUE);
                d2_short = FALSE;
            }
            else if (!strcmp(buf, "0")){
                d2_set(FALSE);
                d2_short = FALSE;
            }
            else if (!strcmp(buf, "2")){
                d2_set(FALSE);
                d2_short = TRUE;
            }
            else {
                printf("d2 parameter must be 0 or 1 or 2.\n");
            }
        }
        else if (!strcmp(buf, "d3")){
            scanf("%s", buf);
            if (!strcmp(buf, "on")){
                d3_set(TRUE);
            }
            else if (!strcmp(buf, "off")){
                d3_set(FALSE);
            }
            else {
                printf("d3 parameter must be either [on] or [off].\n");
            }
        }
        else if (!strcmp(buf, "d4")){
            scanf("%s", buf);
            if (!strcmp(buf, "on")){
                d4_set(TRUE);
            }
            else if (!strcmp(buf, "off")){
                d4_set(FALSE);
            }
            else {
                printf("d4 parameter must be either [on] or [off].\n");
            }
        }
        else if (!strcmp(buf, "d5")){
            scanf("%s", buf);
            if (!strcmp(buf, "on")){
                d5_disas = TRUE;
            }
            else if (!strcmp(buf, "off")){
                d5_disas = FALSE;
            }
            else {
                printf("d5 parameter must be either [on] or [off].\n");
            }
        }
        else if (!strcmp(buf, "d6")){
            scanf("%s", buf);
            if (!strcmp(buf, "on")){
                d6_set(TRUE);
            }
            else if (!strcmp(buf, "off")){
                d6_set(FALSE);
            }
            else {
                printf("d6 parameter must be either [on] or [off].\n");
            }
        }
        else if (!strcmp(buf, "b")){
            unsigned int val;
            scanf("%x", &val);
            break_point = val;
        }
        else if (!strcmp(buf, "bc")){
            unsigned long long val;
            scanf("%llx", &val);
            break_counter_point = (0x0000ffffffffffffLL & val);
            break_nmi_point = (unsigned short)(val >> 48);
            d5_disas = TRUE;
        }
        else if (!strcmp(buf, "del")){
            break_point = 0;
        }
        else if (!strcmp(buf, "m")){
            unsigned int addr;
            int size;
            scanf("%x", &addr);
            scanf("%d", &size);
            dump_mem(addr, size);
        }
        else if (!strcmp(buf, "mf")){
            unsigned int addr;
            int size;
            char outf[MAXBUF];
            scanf("%x", &addr);
            scanf("%d", &size);
            scanf("%s", outf);
            dump_mem_to_file(addr, size, outf);
        }
        else if (!strcmp(buf, "v")){
            unsigned int addr;
            int size;
            scanf("%x", &addr);
            scanf("%d", &size);
            dump_vram(addr, size);
        }
        else if (!strcmp(buf, "spr")){
            unsigned int addr;
            int size;
            scanf("%x", &addr);
            scanf("%d", &size);
            dump_sprite_ram(addr, size);
        }
        else if (!strcmp(buf, "da")){
            unsigned int addr;
            int size;
            int disas_inst(unsigned short addr);
            scanf("%x", &addr);
            scanf("%d", &size);
            while (size > 0) {
                int l = disas_inst(addr);
                if (l == 0) {
                    printf("...(not an instruction)\n");
                    dump_mem(addr, size);
                    break;
                }
                size -= l;
                addr += l;
            }
        }
        else if (!strcmp(buf, "pshow")){
            dump_ppu_reg();
        }
        else {
            printf("unknown command [%s].\n", buf);
            print_debug();
        }
    }
    //start_cpu_clock();
    return TRUE;
}

void disasm(const char* mnemonic, int addr_mode, unsigned short pc, struct cpu_6502* reg) {

    char full_reg[256];
    if (d5_disas)
        printf("%04x%012lx ", (0xffff & get_nmi_cnt()),
        (unsigned long)(0x0000ffffffffffffff & get_clock_cnt()));

    if (d2_short == TRUE) {
        sprintf (full_reg, "%02x, %02x, %02x, %02x, %02x", reg->acc, reg->x, reg->y, reg->sp, *(unsigned char*)&reg->status);
    }
    else {
        full_reg[0] = '\0';
    }
    
    switch(addr_mode) {
        case ADDR_MODE_ZP:
            printf("%04x: %02x %02x       %-5s $%02x %44s\n", pc, 
                    dbg_get_byte(pc), dbg_get_byte(pc + 1), 
                    mnemonic, dbg_get_byte(pc + 1), full_reg);
            break;
        case ADDR_MODE_ZP_X:
            printf("%04x: %02x %02x       %-5s $%02x, x %41s\n", pc, 
                    dbg_get_byte(pc), dbg_get_byte(pc + 1), 
                    mnemonic, dbg_get_byte(pc + 1), full_reg);
            break;
        case ADDR_MODE_ZP_Y:
            printf("%04x: %02x %02x       %-5s $%02x, y %41s\n", pc, 
                    dbg_get_byte(pc), dbg_get_byte(pc + 1), 
                    mnemonic, dbg_get_byte(pc + 1), full_reg);
            break;
        case ADDR_MODE_ABS:
            printf("%04x: %02x %02x %02x    %-5s $%04x %42s\n", pc, 
                    dbg_get_byte(pc), dbg_get_byte(pc + 1), dbg_get_byte(pc + 2), 
                    mnemonic, dbg_get_short(pc + 1), full_reg);
            break;
        case ADDR_MODE_ABS_X:
            printf("%04x: %02x %02x %02x    %-5s $%04x, x %39s\n", pc, 
                    dbg_get_byte(pc), dbg_get_byte(pc + 1), dbg_get_byte(pc + 2), 
                    mnemonic, dbg_get_short(pc + 1), full_reg);
            break;
        case ADDR_MODE_ABS_Y:
            printf("%04x: %02x %02x %02x    %-5s $%04x, y %39s\n", pc, 
                    dbg_get_byte(pc), dbg_get_byte(pc + 1), dbg_get_byte(pc + 2), 
                    mnemonic, dbg_get_short(pc + 1), full_reg);
            break;
        case ADDR_MODE_IND:
            printf("%04x: %02x %02x %02x    %-5s ($%04x) %40s\n", pc, 
                    dbg_get_byte(pc), dbg_get_byte(pc + 1), dbg_get_byte(pc + 2), 
                    mnemonic, dbg_get_short(pc + 1), full_reg);
            break;
        case ADDR_MODE_IMP:
        case ADDR_MODE_ACC:
            printf("%04x: %02x          %-5s %48s\n", pc, 
                    dbg_get_byte(pc), 
                    mnemonic, full_reg);
            break;
        case ADDR_MODE_IMM:
            printf("%04x: %02x %02x       %-5s #$%02x %43s\n", pc, 
                    dbg_get_byte(pc), dbg_get_byte(pc + 1), 
                    mnemonic, dbg_get_byte(pc + 1), full_reg);
            break;
        case ADDR_MODE_REL:
            printf("%04x: %02x %02x       %-5s #%+4d %42s\n", pc, 
                    dbg_get_byte(pc), dbg_get_byte(pc + 1), 
                    mnemonic, (char)dbg_get_byte(pc + 1), full_reg);
            break;
        case ADDR_MODE_INDEX_INDIR:
            printf("%04x: %02x %02x       %-5s ($%02x, x) %39s\n", pc, 
                    dbg_get_byte(pc), dbg_get_byte(pc + 1), 
                    mnemonic, dbg_get_byte(pc + 1), full_reg);
            break;
        case ADDR_MODE_INDIR_INDEX:
            printf("%04x: %02x %02x       %-5s ($%02x), y %39s\n", pc, 
                    dbg_get_byte(pc), dbg_get_byte(pc + 1), 
                    mnemonic, dbg_get_byte(pc + 1), full_reg);
            break;
    }
}

void dump_vram(unsigned short addr, int size) {
    int i;

    if (addr % BYTES_PER_LINE)
        printf("%04x: ", addr % BYTES_PER_LINE);

    for (i = 0; i < addr % BYTES_PER_LINE; i++) {
        printf("   ");
    }
    for (i = 0; i < size; i++) {
        if (addr % BYTES_PER_LINE == 0)
            printf("%04x: ", addr);

        printf("%02x ", vram_data_get(addr));

        if (addr % BYTES_PER_LINE == (BYTES_PER_LINE / 2) - 1)
            printf("  ");

        if (addr % BYTES_PER_LINE == (BYTES_PER_LINE - 1))
            printf("\n");

        addr++;
    }
    printf("\n");
}

void dump_sprite_ram(unsigned short addr, int size) {
    int i;

    if (addr % BYTES_PER_LINE)
        printf("%04x: ", addr % BYTES_PER_LINE);

    for (i = 0; i < addr % BYTES_PER_LINE; i++) {
        printf("   ");
    }
    for (i = 0; i < size; i++) {
        if (addr % BYTES_PER_LINE == 0)
            printf("%04x: ", addr);

        printf("%02x ", spr_ram_tbl_get(addr));

        if (addr % BYTES_PER_LINE == (BYTES_PER_LINE / 2) - 1)
            printf("  ");

        if (addr % BYTES_PER_LINE == (BYTES_PER_LINE - 1))
            printf("\n");

        addr++;
    }
    printf("\n");
}

void dump_mem(unsigned short addr, int size) {
    int i;
    unsigned char vram_data_get(unsigned short addr);

    if (addr % BYTES_PER_LINE)
        printf("%04x: ", addr);

    for (i = 0; i < addr % BYTES_PER_LINE; i++) {
        printf("   ");
        if (i % BYTES_PER_LINE == (BYTES_PER_LINE / 2) - 1)
            printf("  ");
    }
    for (i = 0; i < size; i++) {
        if (addr % BYTES_PER_LINE == 0)
            printf("%04x: ", addr);

        printf("%02x ", dbg_get_byte(addr));

        if (addr % BYTES_PER_LINE == (BYTES_PER_LINE / 2) - 1)
            printf("  ");

        if (addr % BYTES_PER_LINE == (BYTES_PER_LINE - 1))
            printf("\n");

        addr++;
    }
    printf("\n");
}

static void dump_mem_to_file(unsigned short addr, int size, const char* fname) {
    int i;
    unsigned char vram_data_get(unsigned short addr);
    FILE* f_out;

    f_out = fopen(fname, "wb");
    if (f_out == NULL) {
        fprintf(stderr, "output file [%s] open failure.\n", fname);
        return;
    }

    for (i = 0; i < size; i++) {
        char bt = dbg_get_byte(addr);
        fwrite (&bt, 1, 1, f_out);
        addr++;
    }
    
    fclose(f_out);
}

void break_hit(void) {
    printf("------------------\nbreak...\n");
    debug_mode = TRUE;
    dump_6502(TRUE);
    d1_set(TRUE);
}


int init_debug(void) {
    //dprint("init debug..\n");
    debug_history = NULL;
    break_point = 0;
    break_counter_point = 0xffffffffffffffffLL;
    d5_disas = FALSE;
    d2_short = FALSE;
    //initscr();          /* Start curses mode          */
    
    /*initialize mapper debugger.*/
    if (mp_set_debugger != NULL) {
        (*mp_set_debugger)(dbg_rom_get_byte, dbg_rom_get_short);
    }

    return TRUE;
}

void clean_debug(void) {
    //dprint("clean debug..\n");
    //endwin();           /* End curses mode        */
}

