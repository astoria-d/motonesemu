#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include "tools.h"
#include "clock.h"
#include "bus.h"

//typedef int (clock_func_t) (void);
static int fetch_and_decode_inst(void);
static int decode_inst(void);
static int execute_inst(void);
static int reset_handler1(void);
static int reset_handler2(void);

#define NMI_ADDR        0xFFFA
#define RESET_ADDR      0xFFFC
#define IRQ_BRK_ADDR    0xFFFE

void dump_6502(int full);
int decode6502(unsigned char inst);
int execute6502(void);
int test_and_set_exec(void);
int emu_debug(void);
int init_6502core(void);

void pc_set(unsigned short addr);
unsigned short pc_get(void);
void pc_move(int offset);
void start_bus(void);
void end_bus(void);
void set_rw_pin(int rw);

static unsigned char cpu_data_buffer;
static unsigned short cpu_addr_buffer;

/*
 * clock execution function array.
 * max execution cycle is 7.
 * the last element must be NULL.
 * */
clock_func_t *execute_func;

static unsigned long clock_cnt;

/*for debug purpos*/
int get_clock_cnt(void) {
    return clock_cnt;
}


/*
 * clock handler.
 * */
int clock_cpu(void) {
    int ret;

    //dprint("%d\n", clock_cnt);
    clock_cnt++;

    ret = execute_func();

    return ret;
}

unsigned char load_memory(unsigned short addr) {

    set_rw_pin(0);
    set_bus_addr(addr);
    start_bus();
    cpu_data_buffer = get_bus_data();
    end_bus();

    return cpu_data_buffer;
}

void store_memory(unsigned short addr, unsigned char data) {

    set_rw_pin(1);
    set_bus_addr(addr);
    set_bus_data(data);
    start_bus();
    end_bus();

    cpu_addr_buffer = addr;
    cpu_data_buffer = data;
}

/*
 * load address in the memory.
 * loading 2 bytes takes 2 cycles.
 * the parameter "cycle" means first or second read.
 * */
unsigned short load_addr(unsigned short addr, int cycle) {
    unsigned char byte = load_memory(addr);

    ///NES=little endian. lower byte first, higher byte second.
    if (cycle == 1)
        cpu_addr_buffer = byte;
    else
        cpu_addr_buffer |= byte << 8;
    return cpu_addr_buffer;
}

void reset_cpu(void) {
    init_6502core();
    execute_func = reset_handler1;
}

static int reset_handler1(void) {
    load_addr(RESET_ADDR, 1);
    execute_func = reset_handler2;
    return TRUE;
}

static int reset_handler2(void) {
    load_addr(RESET_ADDR + 1, 2);
    pc_set(cpu_addr_buffer);

    execute_func = fetch_and_decode_inst;
    return TRUE;
}

static int decode_inst(void) {
    int ret;

    ret = decode6502(cpu_data_buffer);

    return ret;
}

static int fetch_and_decode_inst(void) {
    int ret;
    extern int debug_mode;


    if (debug_mode) {
        int ret = emu_debug();
        if (!ret)
            return FALSE;
    }
    //dprint("fetch\n");
    load_memory(pc_get());
    //dump_6502(FALSE);

    ret = decode_inst();
    if (!ret) {
        extern int critical_error;
        fprintf(stderr, "cpu decode instruction failure.\n");
        critical_error = TRUE;
        //raise(SIGINT);
        //abort();
        return FALSE;
    }

    execute_func = execute_inst;
    pc_move(1);

    return TRUE;
}

static int execute_inst(void) {
    int ret;
    extern int critical_error;

    //dprint("execute\n");

    //execute the instruction
    ret = execute6502();
    /*
    */
    if (!ret) {
        fprintf(stderr, "cpu execute instruction failure.\n");
        critical_error = TRUE;
        //raise(SIGINT);
        return ret;
    }

    //last cycle goes to next execute cycle
    if (test_and_set_exec()) {
        execute_func = fetch_and_decode_inst;
    }
    return TRUE;
}

void set_cpu_addr_buf(unsigned short addr) {
    cpu_addr_buffer = addr;
}

unsigned short get_cpu_addr_buf(void) {
    return cpu_addr_buffer;
}

void set_cpu_data_buf(unsigned char data) {
    cpu_data_buffer = data;
}

unsigned char get_cpu_data_buf(void) {
    return cpu_data_buffer;
}

int init_cpu(void) {
    int ret;

    ret = init_6502core();
    if (!ret) {
        return FALSE;
    }

    ret = register_clock_hander(clock_cpu);
    if (!ret) {
        return FALSE;
    }
    execute_func = NULL;
    clock_cnt = 0;
    cpu_data_buffer = 0;
    cpu_addr_buffer = 0;

    return TRUE;
}

