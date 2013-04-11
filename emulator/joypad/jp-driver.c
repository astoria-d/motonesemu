#include <stdio.h>

#include "tools.h"

int init_joypad_wnd(void);
void* window_start(void* arg);

int main(int argc, char** argv) {
    printf("jp-driver...\n");

    init_joypad_wnd();
    window_start(NULL);
    
    return 0;
}


