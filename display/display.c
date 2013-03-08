#include <stdio.h>

#include "tools.h"

int window_start(int argc, char** argv);
int window_init(void);


int main(int argc, char** argv) {
    int ret;
    dprint("starting %s...\n", argv[0]);
    
    ret = window_init();
    if (!ret) {
        fprintf(stderr, "window init error.\n");
        return -1;
    }

    return window_start(argc, argv);
}

