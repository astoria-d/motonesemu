#include <stdio.h>
#include "tools.h"
#include "clock.h"

#define MAXBUF  1024

extern int debug_mode;
void dump_6502(int full);

static print_debug(void) {
    printf("command:\n");
    printf(" s: step\n");
    printf(" c: continue\n");
    printf(" show: show registers\n");
    printf(" q: quit emulator\n");
}

int emu_debug(void) {
    char buf[MAXBUF];

    //pause_cpu_clock();
    while (TRUE) {
        fflush(stdin);
        fflush(stdout);
        printf("motonesemu: ");
        scanf("%s", buf);

        if (!strcmp(buf, "s")){
            break;
        }
        else if (!strcmp(buf, "q")){
            extern int critical_error;

            printf("quit...\n");
            debug_mode = FALSE;
            critical_error = TRUE;
            raise(SIGINT);
            return FALSE;
            break;
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

