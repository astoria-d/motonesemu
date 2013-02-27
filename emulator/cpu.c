#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include "tools.h"
#include "clock.h"
#include "bus.h"

unsigned char inst_buf;

struct status_reg {
    unsigned int negative       :1;
    unsigned int overflow       :1;
    unsigned int researved      :1;
    unsigned int break_mode     :1;
    unsigned int decimal        :1;
    unsigned int irq_disable    :1;
    unsigned int zero           :1;
    unsigned int carry          :1;
};

struct cpu_6502 {
    unsigned char acc;
    unsigned char x;
    unsigned char y;
    unsigned short sp;
    struct status_reg status;
    unsigned short pc;
};

static struct cpu_6502 cpu_reg;
static unsigned char cpu_data_buffer;
static unsigned short work_addr_buffer;

//typedef int (clock_func_t) (void);
static int fetch_inst(void);
static int decode_inst(void);
static int execute_inst(void);
static int reset_handler1(void);
static int reset_handler2(void);

static int instruction_cycle;
static int instruction_len;
static int current_cycle;

#define NMI_ADDR        0xFFFA
#define RESET_ADDR      0xFFFC
#define IRQ_BRK_ADDR    0xFFFE

void dump_cpu(int full);
int decode6502(unsigned char inst, int *cycle_cnt, int *inst_len);
int execute6502(void);
int emu_debug(void);
int init_6502core(void);

extern int debug_mode;

/*
 * clock execution function array.
 * max execution cycle is 7.
 * the last element must be NULL.
 * */
clock_func_t *execute_func[8];
int current_exec_func;

unsigned long clock_cnt;

/*
 * clock handler.
 * */
int clock_cpu(void) {
    int ret;

    dprint("---------------\n%d\n", clock_cnt);
    //dump_cpu(0);
    clock_cnt++;

    ret = execute_func[current_exec_func]();

    return ret;
}

static unsigned char load_memory(unsigned short addr) {
    int rw = 0;
    struct timespec ts = {CPU_CLOCK_SEC, CPU_CLOCK_NSEC / 2};

    set_rw_pin(rw);
    set_bus_addr(addr);
    //must wait half cycle for the bus ready
    nanosleep(&ts, NULL);

    cpu_data_buffer = get_bus_data();
    return cpu_data_buffer;
}

/*
 * load address in the memory.
 * loading 2 bytes takes 2 cycles.
 * the parameter "cycle" means first or second read.
 * */
static void load_addr(unsigned short addr, int cycle) {
    unsigned char byte = load_memory(addr);
    if (cycle == 1)
        work_addr_buffer = byte;
    else
        work_addr_buffer |= byte << 8;
}

void reset_cpu(void) {
    memset(&cpu_reg, 0, sizeof(struct cpu_6502));
    execute_func[0] = reset_handler1;
    execute_func[1] = reset_handler2;
    execute_func[2] = NULL;
    current_exec_func = 0;
}

static int reset_handler1(void) {
    load_addr(RESET_ADDR, 1);
    current_exec_func++;
}

static int reset_handler2(void) {
    load_addr(RESET_ADDR + 1, 2);
    cpu_reg.pc = work_addr_buffer;

    execute_func[0] = fetch_inst;
    execute_func[1]= execute_inst;
    execute_func[2] = NULL;
    current_exec_func = 0;
}

static int fetch_inst(void) {
    dprint("fetch\n");
    if (debug_mode) {
        int ret = emu_debug();
        if (!ret)
            return FALSE;
    }
    load_memory(cpu_reg.pc);
    dump_cpu(FALSE);

    cpu_reg.pc++;
    current_exec_func++;
    current_cycle++;
    return TRUE;
}

static int decode_inst(void) {
    int ret;

    ret = decode6502(cpu_data_buffer, &instruction_cycle, &instruction_len);

    if (!ret) 
        return ret;

    cpu_reg.pc += instruction_len - 1;
    return ret;
}

static int execute_inst(void) {
    int ret;
    extern int critical_error;

    dprint("execute\n");

    if (current_cycle == 1) {
        ret = decode_inst();
        if (!ret) {
            fprintf(stderr, "cpu decode instruction failure.\n");
            critical_error = TRUE;
            raise(SIGINT);
            //abort();
            return FALSE;
        }
    }

    ret = execute6502();
    if (!ret) {
        fprintf(stderr, "cpu execute instruction failure.\n");
        critical_error = TRUE;
        raise(SIGINT);
        //abort();
        return ret;
    }

    if (current_cycle == instruction_cycle - 1) {
        execute_func[0] = fetch_inst;
        execute_func[1]= execute_inst;
        execute_func[2] = NULL;
        current_exec_func = 0;
        current_cycle = 0;
    }
    else {
        current_cycle++;
    }
    return TRUE;
}

void dump_cpu(int full) {
    if (full) 
        printf("6502 CPU registers:\n");

    printf("pc:     %04x\n", cpu_reg.pc);
    if (full) {
        printf("acc:    %02x\n", cpu_reg.acc);
        printf("x:      %02x\n", cpu_reg.x);
        printf("y:      %02x\n", cpu_reg.y);
        printf("sp:     %04x\n", cpu_reg.sp);
        printf("status:\n");
        printf(" negative:   %d\n", cpu_reg.status.negative);
        printf(" overflow:   %d\n", cpu_reg.status.overflow);
        printf(" break:      %d\n", cpu_reg.status.break_mode);
        printf(" decimal:    %d\n", cpu_reg.status.decimal);
        printf(" irq:        %d\n", cpu_reg.status.irq_disable);
        printf(" zero:       %d\n", cpu_reg.status.zero);
        printf(" carry:      %d\n", cpu_reg.status.carry);
        printf("-------------------\n");
    }
    printf("data:     %02x\n", cpu_data_buffer);
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
    memset(&cpu_reg, 0, sizeof(struct cpu_6502));
    memset(execute_func,0, sizeof(execute_func));
    current_exec_func = 0;
    clock_cnt = 0;
    cpu_data_buffer = 0;
    work_addr_buffer = 0;

    instruction_cycle = 0;
    instruction_len = 0;
    current_cycle = 0;

    return TRUE;
}

