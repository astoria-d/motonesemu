#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#include "tools.h"
#include "clock.h"
#include "rom.h"
#include "ram.h"

int init_cpu(void);
int init_bus(void);
int init_debug(void);
int init_ppu(void);
int init_apu(void);
int init_dma(void);
int init_joypad(void);

void clean_bus(void);
void clean_debug(void);
void clean_ppu(void);
void clean_apu(void);
void clean_dma(void);
void clean_joypad(void);

void reset_cpu(void);
int load_cartridge(const char* cartridge);

static int main_loop_done;
static int param_debug;
int critical_error;
int debug_mode;

static int init_datas(void) {
    int ret;

    main_loop_done = FALSE;
    critical_error = FALSE;

    ret = init_clock();
    if (!ret) {
        fprintf(stderr, "clock init err.\n");
        return FALSE;
    }

    ret = init_bus();
    if (!ret) {
        fprintf(stderr, "bus init err.\n");
        return FALSE;
    }

    ret = init_rom();
    if (!ret) {
        fprintf(stderr, "rom init err.\n");
        return FALSE;
    }

    ret = init_ram();
    if (!ret) {
        fprintf(stderr, "ram init err.\n");
        return FALSE;
    }

    ret = init_ppu();
    if (!ret) {
        fprintf(stderr, "ppu init err.\n");
        return FALSE;
    }

    ret = init_joypad();
    if (!ret) {
        fprintf(stderr, "ppu init err.\n");
        return FALSE;
    }

    ret = init_apu();
    if (!ret) {
        fprintf(stderr, "apu init err.\n");
        return FALSE;
    }

    ret = init_dma();
    if (!ret) {
        fprintf(stderr, "dma init err.\n");
        return FALSE;
    }

    ret = init_cpu();
    if (!ret) {
        fprintf(stderr, "cpu init err.\n");
        return FALSE;
    }

    ret = init_debug();
    if (!ret) {
        fprintf(stderr, "debug init err.\n");
        return FALSE;
    }

    return TRUE;
}

static void clean_datas(void) {
    dprint("clean data...\n");
    clean_clock();
    clean_dma();
    clean_rom();
    clean_ram();
    clean_ppu();
    clean_joypad();
    clean_apu();
    clean_bus();

    clean_debug();
}

static void sig_handler(int sig) {
    if (!critical_error && param_debug) {
        debug_mode = TRUE;
        return;
    }
    main_loop_done = TRUE;
}

static int prepare_sig(void) {
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

static void print_usage(void) {
    printf("motonesemu [option...] [.nes file]\n");
    printf("Options:\n");
    printf("\t-h: print this page.\n");
    printf("\t-d: debug mode.\n");
    //printf("\t-o [output]: output object file.\n");
}

int main(int argc, char* argv[]) {
    int ret;
    char ch;
    char* cartridge;
    extern int optind;
    param_debug = FALSE;
    printf("motonesemu start...\n");

    while( (ch = getopt(argc, argv, "dh")) != -1) {
        switch (ch) {
            case 'd':
                param_debug = TRUE;
                break;
            case 'h':
            default:
                print_usage();
                return 0;
        }
    }
    argc -= optind - 1;
    argv += optind - 1;

    if (argc <= 1) {
        print_usage();
        return -1;
    }
    debug_mode = param_debug;

    ret = init_datas();
    if (!ret) {
        fprintf(stderr, "initialization failure...\n");
        return RT_ERROR;
    }

    ///register the Ctrl-C signal handler.
    /*
     */
    ret = prepare_sig();
    if (!ret) {
        fprintf(stderr, "signal handling error...\n");
        return RT_ERROR;
    }
 
    cartridge = argv[1];
    ret = load_cartridge(cartridge);
    if (!ret) {
        fprintf(stderr, "load cartridge file err.\n");
        return FALSE;
    }

    reset_cpu();
    ret = start_clock();
    if (!ret) {
        fprintf(stderr, "clock start error\n");
        return FALSE;
    }

    while (!main_loop_done) {
        sleep(1);
        if (critical_error)
            break;
    }

    clean_datas();

    return 0;
}

