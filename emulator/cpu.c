#include <string.h>
#include <stdio.h>
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

int reset6502(void);
int reset_exec6502(void);
int decode6502(unsigned char inst);
int execute6502(void);
int test_and_set_exec(void);
int test_and_set_intr(void);
int emu_debug(void);
int init_6502core(void);
int nmi6502(void);
int bus_ready(void);

void pc_set(unsigned short addr);
unsigned short pc_get(void);
void pc_move(int offset);
void report_exec_err(void);
void d1_set(int on_off);
void d2_set(int on_off);
void d3_set(int on_off);
unsigned int get_nmi_cnt(void);

//for debug.c
void break_hit(void);
int disas_inst(unsigned short addr);
void dump_6502(int full);
extern int debug_mode;
extern unsigned short break_point;
extern unsigned long break_counter_point;
extern unsigned char break_nmi_point;
extern int critical_error;


static unsigned char cpu_data_buffer;
static unsigned short cpu_addr_buffer;

static void (*dump_6502_level2)(int);
static void (*dump_6502_level3_load)(unsigned short, unsigned char);
static void (*dump_6502_level3_store)(unsigned short, unsigned char);
static int (*d1_disas)(unsigned short);

/*
 * clock execution function array.
 * max execution cycle is 7.
 * the last element must be NULL.
 * */
clock_func_t *execute_func;

static unsigned long clock_cnt;

/*for debug purpos*/
unsigned long get_clock_cnt(void) {
    return clock_cnt;
}

void reset_clock_cnt(void) {
    clock_cnt = 0;
}


/*
 * clock handler.
 * */
int clock_cpu(void) {
    int ret;

    if (get_reset_pin()) {
        execute_func = reset_handler1;
        set_reset_pin(0);
        return execute_func();
    }

    ret = execute_func();

    return ret;
}

int register_cpu_clock(void) {
    return register_clock_hander(clock_cpu, CPU_DEVIDER);
}

int unregister_cpu_clock(void) {
    return unregister_clock_hander(clock_cpu);
}

unsigned char load_memory(unsigned short addr) {

    set_rw_pin(0);
    set_bus_addr(addr);
    start_bus();
    cpu_data_buffer = get_bus_data();
    end_bus();

    dump_6502_level3_load (addr, cpu_data_buffer);
    /*
    */
    return cpu_data_buffer;
}

void store_memory(unsigned short addr, unsigned char data) {
    dump_6502_level3_store (addr, data);

    set_rw_pin(1);
    set_bus_addr(addr);
    set_bus_data(data);
    start_bus();
    end_bus();

    cpu_data_buffer = data;
}

/*
 * load address in the memory.
 * loading 2 bytes takes 2 cycles.
 * the parameter "cycle" means first or second read.
 * */
unsigned short load_addr(unsigned short addr, int cycle) {
    unsigned short byte = load_memory(addr);

    ///NES=little endian. lower byte first, higher byte second.
    if (cycle == 1)
        cpu_addr_buffer = ((cpu_addr_buffer & 0xff00) | byte);
    else
        cpu_addr_buffer = ((cpu_addr_buffer & 0x00ff) | (byte << 8));
    return cpu_addr_buffer;
}

static int reset_handler2(void) {
    int ret;
    ret = reset_exec6502();
    if (!ret) {
        fprintf(stderr, "cpu reset failure.\n");
        critical_error = TRUE;
        return FALSE;
    }

    if (test_and_set_intr()) {
        execute_func = fetch_and_decode_inst;
    }
    return ret;
}

static int reset_handler1(void) {
    int ret;
    ret = reset6502();
    if (!ret) {
        fprintf(stderr, "cpu reset failure.\n");
        critical_error = TRUE;
        return FALSE;
    }
    execute_func = reset_handler2;
    return TRUE;
}

void reset_cpu(void) {
    set_reset_pin(1);
}

static int nmi_handler(void) {
    int ret;

    ret = nmi6502();
    if (!ret) {
        dump_6502(TRUE);
        fprintf(stderr, "cpu nmi handling failure.\n");
        while (emu_debug());

        critical_error = TRUE;
        return FALSE;
    }

    //last cycle goes to next execute cycle
    if (test_and_set_intr()) {
        //deassert nmi pin.
        set_nmi_pin(FALSE);
        execute_func = fetch_and_decode_inst;
    }
    return ret;
}

static int decode_inst(void) {
    int ret;

    ret = decode6502(cpu_data_buffer);

    return ret;
}

static int fetch_and_decode_inst(void) {
    int ret;
    unsigned short pc;

    //if nmi occurred, no further execution on the current instruction.
    if (get_nmi_pin()) {
        execute_func = nmi_handler;
        return execute_func();
    }


    dump_6502_level2(TRUE);
    pc = pc_get();

    if (break_point == pc && debug_mode == TRUE) {
        break_hit();
    }
    if ((break_nmi_point == get_nmi_cnt()) && (break_counter_point == clock_cnt) && debug_mode == TRUE) {
        break_hit();
    }

    d1_disas(pc);
    if (debug_mode) {
        ret = emu_debug();
        if (!ret)
            return FALSE;
    }
    //dprint("fetch\n");

    //if bus not ready, do nothing.
    if (!bus_ready())
        return TRUE;

    load_memory(pc);

    ret = decode_inst();
    if (!ret) {
        disas_inst(pc);
        dump_6502(TRUE);
        fprintf(stderr, "cpu decode instruction failure.\n");
        while (emu_debug());

        critical_error = TRUE;
        //raise(SIGINT);
        //abort();
        return FALSE;
    }

    execute_func = execute_inst;
    pc_move(1);

    //dprint("%d\n", clock_cnt);
    clock_cnt++;

    return TRUE;
}

static int execute_inst(void) {
    int ret;

    //dprint("execute\n");

    //execute the instruction
    ret = execute6502();
    /*
    */
    if (!ret) {
        report_exec_err();
        dump_6502(TRUE);
        while (emu_debug());

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

/*null func*/
static void null_dump_6502 (int param) {}
static void null_load_store (unsigned short addr, unsigned char data) {}
static int null_disas(unsigned short addr) {return 0;}

static void dump_load (unsigned short addr, unsigned char data) {
    dprint("                                  ");
    dprint("                                  ");
    dprint("load: @%04x = %02x\n", addr, data);
}
static void dump_store (unsigned short addr, unsigned char data) {
    dprint("                                  ");
    dprint("store: @%04x = %02x\n", addr, data);
}

int init_cpu(void) {
    int ret;

    ret = init_6502core();
    if (!ret) {
        return FALSE;
    }

    ret = register_cpu_clock();
    if (!ret) {
        return FALSE;
    }
    execute_func = NULL;
    clock_cnt = 0;
    cpu_data_buffer = 0;
    cpu_addr_buffer = 0;

    d1_set(debug_mode);
    d2_set(FALSE);
    d3_set(FALSE);

    return TRUE;
}

/*------for debug.c-----*/
void d1_set(int on_off) {
    if (on_off) {
        d1_disas = disas_inst;
    }
    else {
        d1_disas = null_disas;
    }
}

void d2_set(int on_off) {
    if (on_off) {
        dump_6502_level2 = dump_6502;
    }
    else {
        dump_6502_level2 = null_dump_6502;
    }
}

void d3_set(int on_off) {
    if (on_off) {
        dump_6502_level3_load = dump_load;
        dump_6502_level3_store = dump_store;
    }
    else {
        dump_6502_level3_load = null_load_store;
        dump_6502_level3_store = null_load_store;
    }
}

