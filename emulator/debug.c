#include <stdio.h>
#include <string.h>
#include <curses.h>
#include "tools.h"
#include "6502core.h"

#define MAXBUF  1024
#define BYTES_PER_LINE 16

extern int debug_mode;
void dump_6502(int full);
void dump_mem(const char* msg, unsigned short base, 
        unsigned short offset, unsigned char* buf, int size);

#define MAX_HISTORY     10

struct cmd_list {
    struct dlist l;
    char *cmd;
};

static struct cmd_list* debug_history;
static int log_msg;
//global variable.
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
    printf("   log on/off: set log msg on/off\n");
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
                log_msg = TRUE;
            }
            else if (!strcmp(buf, "off")){
                log_msg = FALSE;
            }
            else {
                printf("log parameter must be either [on] or [off].\n");
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
            printf("not supported...\n");
        }
        else if (!strcmp(buf, "v")){
            printf("not supported...\n");
        }
        else if (!strcmp(buf, "pshow")){
            printf("not supported...\n");
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
    unsigned char dbg_get_byte(unsigned short addr);
    unsigned short dbg_get_short(unsigned short addr);

    if (!log_msg)
        return;

    switch(addr_mode) {
        case ADDR_MODE_ZP:
            printf("%04x: %-5s $%02x\n", pc, mnemonic, dbg_get_byte(pc + 1));
            break;
        case ADDR_MODE_ZP_X:
            printf("%04x: %-5s $%02x, x\n", pc, mnemonic, dbg_get_byte(pc + 1));
            break;
        case ADDR_MODE_ZP_Y:
            printf("%04x: %-5s $%02x, y\n", pc, mnemonic, dbg_get_byte(pc + 1));
            break;
        case ADDR_MODE_ABS:
            printf("%04x: %-5s $%04x\n", pc, mnemonic, dbg_get_short(pc + 1));
            break;
        case ADDR_MODE_ABS_X:
            printf("%04x: %-5s $%04x, x\n", pc, mnemonic, dbg_get_short(pc + 1));
            break;
        case ADDR_MODE_ABS_Y:
            printf("%04x: %-5s $%04x, y\n", pc, mnemonic, dbg_get_short(pc + 1));
            break;
        case ADDR_MODE_IND:
            printf("%04x: %-5s ($%04x), y\n", pc, mnemonic, dbg_get_short(pc + 1));
            break;
        case ADDR_MODE_IMP:
        case ADDR_MODE_ACC:
            printf("%04x: %s\n", pc, mnemonic);
            break;
        case ADDR_MODE_IMM:
        case ADDR_MODE_REL:
            printf("%04x: %-5s #$%02x\n", pc, mnemonic, dbg_get_byte(pc + 1));
            break;
        case ADDR_MODE_INDEX_INDIR:
            printf("%04x: %-5s ($%02x, x), y\n", pc, mnemonic, dbg_get_byte(pc + 1));
            break;
        case ADDR_MODE_INDIR_INDEX:
            printf("%04x: %-5s ($%02x), y\n", pc, mnemonic, dbg_get_byte(pc + 1));
            break;
    }
}

void dump_mem(const char* msg, unsigned short base, 
        unsigned short offset, unsigned char* buf, int size) {
    int i;

    printf(msg);
    if (offset % BYTES_PER_LINE)
        printf("%04x: ", base + offset % BYTES_PER_LINE);

    for (i = 0; i < offset % BYTES_PER_LINE; i++) {
        printf("   ");
    }
    for (i = 0; i < size; i++) {
        if (offset % BYTES_PER_LINE == 0)
            printf("%04x: ", base + offset);

        printf("%02x ", *buf);

        if (offset % BYTES_PER_LINE == (BYTES_PER_LINE / 2) - 1)
            printf("  ");

        if (offset % BYTES_PER_LINE == (BYTES_PER_LINE - 1))
            printf("\n");

        buf++;
        offset++;
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
    log_msg = debug_mode;
    break_point = 0;
    //initscr();          /* Start curses mode          */

    return TRUE;
}

void clean_debug(void) {
    dprint("clean debug..\n");
    //endwin();           /* End curses mode        */
}

