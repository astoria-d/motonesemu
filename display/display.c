#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "tools.h"
#include "vga.h"

int window_start(int argc, char** argv);
int comm_init(void);
int window_init(void);

int main_file(void) {
    int ret;
    FILE* fifo;
    char buf[100];
    int cnt = 0;

    //create named fifo.
    ret = mknod(VGA_FIFO, S_IFIFO | 0666, 0);
    if (ret != RT_OK && errno != EEXIST) {
        fprintf(stderr, "error creating pipe!\n");
        return -1;
    }

    while (1) {
    dprint("fifo open...");
    
    fifo= fopen(VGA_FIFO, "r");
    if (fifo == NULL) {
        fprintf(stderr, "error opening fifo!\n");
        return -1;
    }
    dprint(" ok.\n");


        int i;

        dprint("cnt: %d\n", cnt++);
        fflush(stdout);
        ret = fread (buf, 1, sizeof(buf), fifo);
        if (ret == 0)
            continue;

        for (i = 0; i < sizeof(buf); i++) {
            printf ("%02x ", buf[i]);
            fflush(stdout);
        }
        dprint("\n");

    dprint("fd close...");
    fclose(fifo);
    dprint(" ok.\n");

    }
    return 0;
}


int main(int argc, char** argv) {
    int ret;
    dprint("starting %s...\n", argv[0]);
    
    ret = window_init();
    if (!ret) {
        fprintf(stderr, "window init error.\n");
        return -1;
    }

    ret = comm_init();
    if (!ret) {
        fprintf(stderr, "comm init error.\n");
        return -1;
    }
    //return main_file();
    return window_start(argc, argv);
}

