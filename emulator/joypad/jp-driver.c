#include <stdio.h>

#include "tools.h"

int init_jp(void);
int window_start(int argc, char** argv);

int main(int argc, char** argv) {
    printf("jp-driver...\n");

    init_jp();
    window_start(argc, argv);
    
    return 0;
}


