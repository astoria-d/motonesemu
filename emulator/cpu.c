#include "tools.h"
#include "clock.h"


/*
 * clock handler.
 * */
int clock_cpu(void) {
    dprint("clock cpu...\n");

    return TRUE;
}

void reset_cpu(void) {
}

static void fetch_inst(void) {
}

static void decode_inst(void) {
}

static void execute_inst(void) {
}

int init_cpu(void) {
    int ret;
    ret = register_clock_hander(clock_cpu);
    if (!ret) {
        return FALSE;
    }

    return TRUE;
}

