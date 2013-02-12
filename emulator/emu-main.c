#include <stdio.h>
#include "tools.h"
#include "clock.h"

static int init_datas(void) {
    int ret;

    ret = init_clock();
    if (!ret) {
        fprintf(stderr, "clock init err.\n");
        return FALSE;
    }

    return TRUE;
}

int main(int argc, char* argv[]) {
    int ret;
    printf("motonesemu start...\n");

    ret = init_datas();
    if (!ret) {
        fprintf(stderr, "initialization failure...\n");
        return RT_ERROR;
    }

    clock_loop();

    return 0;
}

