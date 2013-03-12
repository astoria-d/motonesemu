#include <stdio.h>
#include <string.h>
#include <curses.h>
#include "tools.h"

#define MAXBUF  1024
#define BYTES_PER_LINE 16

extern int debug_mode;
void dump_6502(int full);

#define MAX_HISTORY     10

struct cmd_list {
    struct dlist l;
    char *cmd;
};

struct cmd_list* debug_history;

static void print_debug(void) {
    printf("command:\n");
    printf(" s: step\n");
    printf(" c: continue\n");
    printf(" show: show registers\n");
    printf(" q: quit emulator\n");
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
        else {
            printf("unknown command [%s].\n", buf);
            print_debug();
        }
    }
    //start_cpu_clock();
    return TRUE;
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


int init_debug(void) {
    dprint("init debug..\n");
    debug_history = NULL;
    //initscr();          /* Start curses mode          */

    return TRUE;
}

void clean_debug(void) {
    dprint("clean debug..\n");
    //endwin();           /* End curses mode        */
}

