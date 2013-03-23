#include <stdio.h>
#include <string.h>
#include <curses.h>
#include "tools.h"
#include "6502core.h"

#define MAXBUF  1024
#define BYTES_PER_LINE 16

extern int debug_mode;
void dump_6502(int full);
void dump_vram(unsigned short addr, int size);
void dump_mem(unsigned short addr, int size);
unsigned char vram_data_get(unsigned short addr);
unsigned char dbg_get_byte(unsigned short addr);
unsigned short dbg_get_short(unsigned short addr);
void dump_ppu_reg(void);
void d2_set(int on_off);
void d3_set(int on_off);
void d4_set(int on_off);

#define MAX_HISTORY     10

struct cmd_list {
    struct dlist l;
    char *cmd;
};

static struct cmd_list* debug_history;
//global variable.
int dbg_log_msg;
unsigned short break_point;

static void print_debug(void) {
    printf("   command:\n");
    printf("            s: step\n");
    printf("            c: continue\n");
    printf("       b addr: set break point\n");
    printf("               (break point can be set only 1 address.)\n");
    printf("          del: delete break point\n");
    printf("  m addr size: memory dump\n");
    printf("         show: show registers\n");
    printf("        pshow: show ppu registers\n");
    printf("  v addr size: vram dump\n");
    printf("  da addr size: disassemble\n");
    printf("   log on/off: set log msg on/off\n");
    printf("   d2 on/off: debug log level 2 (dump reg status for each instruction)\n");
    printf("   d3 on/off: debug log level 3 (dump load/store data value)\n");
    printf("   d4 on/off: debug log level 4 (dump vram data write)\n");
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
        else if (!strcmp(buf, "show")){
            dump_6502(TRUE);
        }
        else if (!strcmp(buf, "log")){
            scanf("%s", buf);
            if (!strcmp(buf, "on")){
                dbg_log_msg = TRUE;
            }
            else if (!strcmp(buf, "off")){
                dbg_log_msg = FALSE;
            }
            else {
                printf("log parameter must be either [on] or [off].\n");
            }
        }
        else if (!strcmp(buf, "d2")){
            scanf("%s", buf);
            if (!strcmp(buf, "on")){
                d2_set(TRUE);
            }
            else if (!strcmp(buf, "off")){
                d2_set(FALSE);
            }
            else {
                printf("d2 parameter must be either [on] or [off].\n");
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
        else if (!strcmp(buf, "b")){
            unsigned int val;
            scanf("%x", &val);
            break_point = val;
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
        else if (!strcmp(buf, "v")){
            unsigned int addr;
            int size;
            scanf("%x", &addr);
            scanf("%d", &size);
            dump_vram(addr, size);
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

void disasm(const char* mnemonic, int addr_mode, unsigned short pc) {

    switch(addr_mode) {
        case ADDR_MODE_ZP:
            printf("%04x: %02x %02x       %-5s $%02x\n", pc, 
                    dbg_get_byte(pc), dbg_get_byte(pc + 1), 
                    mnemonic, dbg_get_byte(pc + 1));
            break;
        case ADDR_MODE_ZP_X:
            printf("%04x: %02x %02x       %-5s $%02x, x\n", pc, 
                    dbg_get_byte(pc), dbg_get_byte(pc + 1), 
                    mnemonic, dbg_get_byte(pc + 1));
            break;
        case ADDR_MODE_ZP_Y:
            printf("%04x: %02x %02x       %-5s $%02x, y\n", pc, 
                    dbg_get_byte(pc), dbg_get_byte(pc + 1), 
                    mnemonic, dbg_get_byte(pc + 1));
            break;
        case ADDR_MODE_ABS:
            printf("%04x: %02x %02x %02x    %-5s $%04x\n", pc, 
                    dbg_get_byte(pc), dbg_get_byte(pc + 1), dbg_get_byte(pc + 2), 
                    mnemonic, dbg_get_short(pc + 1));
            break;
        case ADDR_MODE_ABS_X:
            printf("%04x: %02x %02x %02x    %-5s $%04x, x\n", pc, 
                    dbg_get_byte(pc), dbg_get_byte(pc + 1), dbg_get_byte(pc + 2), 
                    mnemonic, dbg_get_short(pc + 1));
            break;
        case ADDR_MODE_ABS_Y:
            printf("%04x: %02x %02x %02x    %-5s $%04x, y\n", pc, 
                    dbg_get_byte(pc), dbg_get_byte(pc + 1), dbg_get_byte(pc + 2), 
                    mnemonic, dbg_get_short(pc + 1));
            break;
        case ADDR_MODE_IND:
            printf("%04x: %02x %02x       %-5s ($%04x), y\n", pc, 
                    dbg_get_byte(pc), dbg_get_byte(pc + 1), 
                    mnemonic, dbg_get_short(pc + 1));
            break;
        case ADDR_MODE_IMP:
        case ADDR_MODE_ACC:
            printf("%04x: %02x          %-5s \n", pc, 
                    dbg_get_byte(pc), 
                    mnemonic);
            break;
        case ADDR_MODE_IMM:
            printf("%04x: %02x %02x       %-5s #$%02x\n", pc, 
                    dbg_get_byte(pc), dbg_get_byte(pc + 1), 
                    mnemonic, dbg_get_byte(pc + 1));
            break;
        case ADDR_MODE_REL:
            printf("%04x: %02x %02x       %-5s #%+d\n", pc, 
                    dbg_get_byte(pc), dbg_get_byte(pc + 1), 
                    mnemonic, (char)dbg_get_byte(pc + 1));
            break;
        case ADDR_MODE_INDEX_INDIR:
            printf("%04x: %02x %02x       %-5s ($%02x, x), y\n", pc, 
                    dbg_get_byte(pc), dbg_get_byte(pc + 1), 
                    mnemonic, dbg_get_byte(pc + 1));
            break;
        case ADDR_MODE_INDIR_INDEX:
            printf("%04x: %02x %02x       %-5s ($%02x), y\n", pc, 
                    dbg_get_byte(pc), dbg_get_byte(pc + 1), 
                    mnemonic, dbg_get_byte(pc + 1));
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

void break_hit(void) {
    printf("------------------\nbreak...\n");
    debug_mode = TRUE;
    dump_6502(TRUE);
}


int init_debug(void) {
    dprint("init debug..\n");
    debug_history = NULL;
    dbg_log_msg = debug_mode;
    break_point = 0;
    //initscr();          /* Start curses mode          */

    return TRUE;
}

void clean_debug(void) {
    dprint("clean debug..\n");
    //endwin();           /* End curses mode        */
}

