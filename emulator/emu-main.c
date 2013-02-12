#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#include "tools.h"
#include "clock.h"

int init_cpu(void);
int init_bus(void);
void clean_bus(void);
void reset_cpu(void);

static int clock_done;

static int init_datas(void) {
    int ret;

    clock_done = FALSE;

    ret = init_bus();
    if (!ret) {
        fprintf(stderr, "bus init err.\n");
        return FALSE;
    }

    ret = init_clock();
    if (!ret) {
        fprintf(stderr, "clock init err.\n");
        return FALSE;
    }

    ret = init_cpu();
    if (!ret) {
        fprintf(stderr, "cpu init err.\n");
        return FALSE;
    }

    return TRUE;
}

static void clean_datas(void) {
    dprint("clean data...\n");
    clean_clock();
    clean_bus();
}

static void sig_handler(int sig) {
    clock_done = TRUE;
}

static int prepare_sig(void) {
    int ret;
    struct sigaction sigact;

    memset(&sigact, 0, sizeof(struct sigaction));
    sigact.sa_handler = sig_handler;
    if ( sigemptyset(&sigact.sa_mask) ) {
        return FALSE;
    }
    if ( sigaction(SIGINT, &sigact, NULL) ) {
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

    ///register the Ctrl-C signal handler.
    ret = prepare_sig();
    if (!ret) {
        fprintf(stderr, "signal handling error...\n");
        return RT_ERROR;
    }
 
    reset_cpu();
    start_clock();

    while (!clock_done) {
        sleep(1);
    }

    clean_datas();

    return 0;
}

