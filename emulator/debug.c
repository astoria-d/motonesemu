#include <stdio.h>
#include "tools.h"
#include "clock.h"

#define MAXBUF  1024

extern int debug_mode;

static print_debug(void) {
    printf("command:\n");
    printf(" s: step\n");
    printf(" c: continue\n");
}

void emu_debug(void) {
    char buf[MAXBUF];
    int done = FALSE;

    pause_cpu_clock();
    while (!done) {
    printf("motonesemu: ");
    scanf("%s", buf);

        if (!strcmp(buf, "s")){
            break;
        }
        else if (!strcmp(buf, "c")){
            debug_mode = FALSE;
            break;
        }
        else {
            printf("unknown command.\n");
            print_debug();
        }
    }
    start_cpu_clock();
}

