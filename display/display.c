#include <stdio.h>

#include "tools.h"

int window_start(int argc, char** argv);
int comm_init(void);
int receiver_init(void);
int window_init(void);


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

