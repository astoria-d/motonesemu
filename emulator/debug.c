#include <stdio.h>
#include <string.h>
#include "tools.h"
#include "clock.h"

#define MAXBUF  1024

extern int debug_mode;
void dump_6502(int full);

#define MAX_HISTORY     10

struct cmd_list {
    struct dlist l;
    char *cmd;
};

struct cmd_list* debug_history;

static print_debug(void) {
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
    while( (ch = getch()) != '\n') {
        dprint("ch:%d\n", ch);
        *p = ch;
        p++;
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
            printf("unknown command.\n");
            print_debug();
        }
    }
    //start_cpu_clock();
    return TRUE;
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

