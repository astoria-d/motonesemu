#include <stdio.h>
#include <string.h>
#ifndef __CYGWIN__
#include <libio.h>
#endif
#include "tools.h"
#include "6502core.h"

typedef int (handler_6502_t) (void);

/*
 * 6502 instructions
 * adressing mode        instruction length
 * 0:Zero Page           2
 * 1:Zero Page, X        2
 * 2:Zero Page, Y        2
 * 3:Absolute            3
 * 4:Absolute, X         3
 * 5:Absolute, Y         3
 * 6:Indirect            3
 * 7:Implied             1
 * 8:Accumulator         1
 * 9:Immediate           2
 * 10:Relative           2
 * 11:(Indirect, X)      2
 * 12:(Indirect), Y      2
 * 
 **/

#define N_BIT      0x80
#define V_BIT      0x40

//stack pointer base address
#define STACK_BASE  0x100


/* cycle check must be cleared on release  */
#define cycle_check


#define NMI_VECTOR        0xFFFA
#define RESET_VECTOR      0xFFFC
#define IRQ_VECTOR        0xFFFE

struct opcode_map {
    unsigned char   opcode;
    char            mnemonic[4];
    handler_6502_t  *func;
    int             addr_mode;
    int             cycle;
    int             cycle_aux;  /*Add one cycle if indexing across page boundary*/
    int             inst_len;
};

static struct cpu_6502 cpu_reg;
static struct opcode_map *current_inst;

//exec index is the cycle consumed for execution only.
//cpu total cycle is current_exec_index + 1 because 
//fetch cycle uses 1 cycle prior to the execution.
static int current_exec_index;
static int exec_done;
static int intr_done;
static int bus_status;

//debug purpose..
static unsigned int nmi_cnt;

unsigned char load_memory(unsigned short addr);
unsigned short load_addr(unsigned short addr, int cycle);
void store_memory(unsigned short addr, unsigned char data);
int bus_ready(void);
#if 0
#define BUS_READY_CHECK()  \
    if ((bus_status = bus_ready()) != TRUE) { return FALSE; } 
#else
//for the performance reason, skip bus ready bit.
#define BUS_READY_CHECK() 
#endif

unsigned char get_cpu_data_buf(void);
void set_cpu_data_buf(unsigned char data);
unsigned short get_cpu_addr_buf(void);
void set_cpu_addr_buf(unsigned short addr);
unsigned long get_clock_cnt(void);
void reset_clock_cnt(void);

int func_ADC(void);
int func_AND(void);
int func_ASL(void);
int func_BCC(void);
int func_BCS(void);
int func_BEQ(void);
int func_BIT(void);
int func_BMI(void);
int func_BNE(void);
int func_BPL(void);
int func_BRK(void);
int func_BVC(void);
int func_BVS(void);
int func_CLC(void);
int func_CLD(void);
int func_CLI(void);
int func_CLV(void);
int func_CMP(void);
int func_CPX(void);
int func_CPY(void);
int func_DEC(void);
int func_DEX(void);
int func_DEY(void);
int func_EOR(void);
int func_INC(void);
int func_INX(void);
int func_INY(void);
int func_JMP(void);
int func_JSR(void);
int func_LDA(void);
int func_LDX(void);
int func_LDY(void);
int func_LSR(void);
int func_NOP(void);
int func_ORA(void);
int func_PHA(void);
int func_PHP(void);
int func_PLA(void);
int func_PLP(void);
int func_ROL(void);
int func_ROR(void);
int func_RTI(void);
int func_RTS(void);
int func_SBC(void);
int func_SEC(void);
int func_SED(void);
int func_SEI(void);
int func_STA(void);
int func_STX(void);
int func_STY(void);
int func_TAX(void);
int func_TAY(void);
int func_TSX(void);
int func_TXA(void);
int func_TXS(void);
int func_TYA(void);

struct opcode_map opcode_list [255] = {
#include "opcode"
};


/*
 * awk '{print "int func_" $2 "(void) {\n\n}"}' < opcode-6502 | sort | uniq
 *
 * */



/*
 * load from memory in various addressing mode.
 * cpu_data_buf has the output value.
 *
 *
 * */
static int load_addr_mode(int *done) {
    BUS_READY_CHECK()

    switch (current_inst->addr_mode) {
        case ADDR_MODE_ACC:
        case ADDR_MODE_IMP:
            //not supported.
            return FALSE;

        case ADDR_MODE_IMM:
        case ADDR_MODE_REL:
            //load immediate/relative value takes 1 cycle.
            if (current_exec_index == 0) {
                load_memory(cpu_reg.pc);
                cpu_reg.pc++;
                goto addr_mode_done;
            }
            break;

        case ADDR_MODE_ZP:
            //zp takes two cycles.
            if (current_exec_index == 0) {
                load_memory(cpu_reg.pc);
                cpu_reg.pc++;
                return TRUE;
            }
            else if (current_exec_index == 1) {
                unsigned short zp = get_cpu_data_buf();
                load_memory(zp);
                goto addr_mode_done;
            }
            break;

            /*
             * Indexed zero page
             * Wraparound is used when performing the addition so 
             * the address of the data will always be
             * in zero page. For example, if the operand is $FF and 
             * the X register contains $01 the address
             * of the data will be $0000, not $0100.
             * */
        case ADDR_MODE_ZP_X:
            //zp indexed with x takes three cycles.
            if (current_exec_index == 0) {
                load_memory(cpu_reg.pc);
                cpu_reg.pc++;
                return TRUE;
            }
            else if (current_exec_index == 1) {
                unsigned char imm = get_cpu_data_buf();
                set_cpu_data_buf(imm + cpu_reg.x);
                return TRUE;
            }
            else if (current_exec_index == 2) {
                unsigned char zp = get_cpu_data_buf();
                load_memory(zp);
                goto addr_mode_done;
            }
            break;

        case ADDR_MODE_ZP_Y:
            //zp indexed with y takes three cycles.
            if (current_exec_index == 0) {
                load_memory(cpu_reg.pc);
                cpu_reg.pc++;
                return TRUE;
            }
            else if (current_exec_index == 1) {
                unsigned char imm = get_cpu_data_buf();
                set_cpu_data_buf(imm + cpu_reg.y);
                return TRUE;
            }
            else if (current_exec_index == 2) {
                unsigned char zp = get_cpu_data_buf();
                load_memory(zp);
                goto addr_mode_done;
            }
            break;

        case ADDR_MODE_ABS:
            //takes three cycles.
            if (current_exec_index == 0) {
                load_addr(cpu_reg.pc, 1);
                cpu_reg.pc++;
                return TRUE;
            }
            else if (current_exec_index == 1) {
                load_addr(cpu_reg.pc, 2);
                cpu_reg.pc++;
                return TRUE;
            }
            else if (current_exec_index == 2) {
                unsigned short addr = get_cpu_addr_buf();
                load_memory(addr);
                goto addr_mode_done;
            }
            break;

        case ADDR_MODE_ABS_X:
            //abs indexed with x takes three cycles.
            if (current_exec_index == 0) {
                load_addr(cpu_reg.pc, 1);
                cpu_reg.pc++;
                return TRUE;
            }
            else if (current_exec_index == 1) {
                load_addr(cpu_reg.pc, 2);
                cpu_reg.pc++;
                return TRUE;
            }
            else if (current_exec_index == 2) {
                if (current_inst->cycle_aux) {
                    //Add one cycle if indexing across page boundary
                    unsigned short addr = get_cpu_addr_buf();
                    unsigned short hi_8, added_hi_8;

                    hi_8 = (addr >> 8);
                    addr += cpu_reg.x;
                    added_hi_8 = (addr >> 8);

                    if (hi_8 == added_hi_8) {
                        load_memory(addr);
                        goto addr_mode_done;
                    }

                    //load value in the next cycle.
                    set_cpu_addr_buf(addr);
                    return TRUE;
                }
                else {
                    unsigned short addr = get_cpu_addr_buf();
                    addr += cpu_reg.x;
                    load_memory(addr);
                    goto addr_mode_done;
                }
            }
            else if (current_exec_index == 3) {
                if (current_inst->cycle_aux) {
                    unsigned short addr = get_cpu_addr_buf();
                    load_memory(addr);
                    goto addr_mode_done;
                }
            }

            break;

        case ADDR_MODE_ABS_Y:
            //abs indexed with y takes three cycles.
            if (current_exec_index == 0) {
                load_addr(cpu_reg.pc, 1);
                cpu_reg.pc++;
                return TRUE;
            }
            else if (current_exec_index == 1) {
                load_addr(cpu_reg.pc, 2);
                cpu_reg.pc++;
                return TRUE;
            }
            else if (current_exec_index == 2) {
                if (current_inst->cycle_aux) {
                    //Add one cycle if indexing across page boundary
                    unsigned short addr = get_cpu_addr_buf();
                    unsigned short hi_8, added_hi_8;

                    hi_8 = (addr >> 8);
                    addr += cpu_reg.y;
                    added_hi_8 = (addr >> 8);

                    if (hi_8 == added_hi_8) {
                        load_memory(addr);
                        goto addr_mode_done;
                    }

                    //load value in the next cycle.
                    set_cpu_addr_buf(addr);
                    return TRUE;
                }
                else {
                    unsigned short addr = get_cpu_addr_buf();
                    addr += cpu_reg.y;
                    load_memory(addr);
                    goto addr_mode_done;
                }
            }
            else if (current_exec_index == 3) {
                if (current_inst->cycle_aux) {
                    unsigned short addr = get_cpu_addr_buf();
                    load_memory(addr);
                    goto addr_mode_done;
                }
            }
            break;

        case ADDR_MODE_INDEX_INDIR:
            //Zero Page Indexed Indirect: (zp,x) takes 5 cycles
            if (current_exec_index == 0) {
                load_memory(cpu_reg.pc);
                cpu_reg.pc++;
                return TRUE;
            }
            else if (current_exec_index == 1) {
                unsigned char zp = get_cpu_data_buf();
                zp += cpu_reg.x;
                set_cpu_data_buf(zp);
                return TRUE;
            }
            else if (current_exec_index == 2) {
                unsigned char addr = get_cpu_data_buf();
                load_addr(addr, 1);
                //save addr in the temporary buffer.
                set_cpu_data_buf(addr);
                return TRUE;
            }
            else if (current_exec_index == 3) {
                unsigned char addr = get_cpu_data_buf();
                load_addr((unsigned char)(addr + 1), 2);
                return TRUE;
            }
            else if (current_exec_index == 4) {
                load_memory(get_cpu_addr_buf());
                goto addr_mode_done;
            }
            break;

        case ADDR_MODE_INDIR_INDEX:
            //Zero Page Indirect Indexed with Y: (zp),y takes 4 cycles.
            if (current_exec_index == 0) {
                load_memory(cpu_reg.pc);
                cpu_reg.pc++;
                return TRUE;
            }
            else if (current_exec_index == 1) {
                unsigned char addr = get_cpu_data_buf();
                load_addr(addr, 1);
                //save addr in the temporary buffer.
                set_cpu_data_buf(addr);
                return TRUE;
            }
            else if (current_exec_index == 2) {
                unsigned char addr = get_cpu_data_buf();
                load_addr((unsigned char)(addr + 1), 2);
                return TRUE;
            }
            else if (current_exec_index == 3) {
                if (current_inst->cycle_aux) {
                    //Add one cycle if indexing across page boundary
                    unsigned short addr = get_cpu_addr_buf();
                    unsigned short hi_8, added_hi_8;

                    hi_8 = (addr >> 8);
                    addr += cpu_reg.y;
                    added_hi_8 = (addr >> 8);

                    if (hi_8 == added_hi_8) {
                        load_memory(addr);
                        goto addr_mode_done;
                    }

                    //load value in the next cycle.
                    set_cpu_addr_buf(addr);
                    return TRUE;
                }
                else {
                    unsigned short addr = get_cpu_addr_buf();
                    addr += cpu_reg.y;
                    load_memory(addr);
                    goto addr_mode_done;
                }
            }
            else if (current_exec_index == 4) {
                if (current_inst->cycle_aux) {
                    unsigned short addr = get_cpu_addr_buf();
                    load_memory(addr);
                    goto addr_mode_done;
                }
            }
            break;

        default:
            return FALSE;
    }
    return FALSE;

addr_mode_done:
    *done = TRUE;
    return TRUE;
}

/*
 * store into memory in various addressing mode.
 * */
static int store_addr_mode(unsigned char data, int *done) {
    BUS_READY_CHECK()

    switch (current_inst->addr_mode) {
        case ADDR_MODE_ACC:
        case ADDR_MODE_IMP:
        case ADDR_MODE_IMM:
        case ADDR_MODE_REL:
            //not supported.
            return FALSE;

        case ADDR_MODE_ZP:
            //zp takes two cycles.
            if (current_exec_index == 0) {
                load_memory(cpu_reg.pc);
                cpu_reg.pc++;
                return TRUE;
            }
            else if (current_exec_index == 1) {
                unsigned short zp = get_cpu_data_buf();
                store_memory(zp, data);
                goto addr_mode_done;
            }
            break;

        case ADDR_MODE_ZP_X:
            //zp indexed with x takes three cycles.
            if (current_exec_index == 0) {
                load_memory(cpu_reg.pc);
                cpu_reg.pc++;
                return TRUE;
            }
            else if (current_exec_index == 1) {
                unsigned char imm = get_cpu_data_buf();
                set_cpu_data_buf(imm + cpu_reg.x);
                return TRUE;
            }
            else if (current_exec_index == 2) {
                unsigned char zp = get_cpu_data_buf();
                store_memory(zp, data);
                goto addr_mode_done;
            }
            break;

        case ADDR_MODE_ZP_Y:
            //zp indexed with y takes three cycles.
            if (current_exec_index == 0) {
                load_memory(cpu_reg.pc);
                cpu_reg.pc++;
                return TRUE;
            }
            else if (current_exec_index == 1) {
                unsigned char imm = get_cpu_data_buf();
                set_cpu_data_buf(imm + cpu_reg.y);
                return TRUE;
            }
            else if (current_exec_index == 2) {
                unsigned char zp = get_cpu_data_buf();
                store_memory(zp, data);
                goto addr_mode_done;
            }
            break;

        case ADDR_MODE_ABS:
            //takes three cycles.
            if (current_exec_index == 0) {
                load_addr(cpu_reg.pc, 1);
                cpu_reg.pc++;
                return TRUE;
            }
            else if (current_exec_index == 1) {
                load_addr(cpu_reg.pc, 2);
                cpu_reg.pc++;
                return TRUE;
            }
            else if (current_exec_index == 2) {
                unsigned short addr = get_cpu_addr_buf();
                store_memory(addr, data);
                goto addr_mode_done;
            }
            break;

        case ADDR_MODE_ABS_X:
            //abs indexed with x takes 4 cycles.
            if (current_exec_index == 0) {
                load_addr(cpu_reg.pc, 1);
                cpu_reg.pc++;
                return TRUE;
            }
            else if (current_exec_index == 1) {
                load_addr(cpu_reg.pc, 2);
                cpu_reg.pc++;
                return TRUE;
            }
            else if (current_exec_index == 2) {
                unsigned short addr = get_cpu_addr_buf();
                addr += cpu_reg.x;
                set_cpu_addr_buf(addr);
                return TRUE;
            }
            else if (current_exec_index == 3) {
                unsigned short addr = get_cpu_addr_buf();
                store_memory(addr, data);
                goto addr_mode_done;
            }
            break;

        case ADDR_MODE_ABS_Y:
            if (current_exec_index == 0) {
                load_addr(cpu_reg.pc, 1);
                cpu_reg.pc++;
                return TRUE;
            }
            else if (current_exec_index == 1) {
                load_addr(cpu_reg.pc, 2);
                cpu_reg.pc++;
                return TRUE;
            }
            else if (current_exec_index == 2) {
                unsigned short addr = get_cpu_addr_buf();
                addr += cpu_reg.y;
                set_cpu_addr_buf(addr);
                return TRUE;
            }
            else if (current_exec_index == 3) {
                unsigned short addr = get_cpu_addr_buf();
                store_memory(addr, data);
                goto addr_mode_done;
            }
            break;

        case ADDR_MODE_INDEX_INDIR:
            //Zero Page Indexed Indirect: (zp,x) takes 5 cycles
            if (current_exec_index == 0) {
                load_memory(cpu_reg.pc);
                cpu_reg.pc++;
                return TRUE;
            }
            else if (current_exec_index == 1) {
                unsigned char zp = get_cpu_data_buf();
                zp += cpu_reg.x;
                set_cpu_data_buf(zp);
                return TRUE;
            }
            else if (current_exec_index == 2) {
                unsigned char addr = get_cpu_data_buf();
                load_addr(addr, 1);
                //save addr in the temporary buffer.
                set_cpu_data_buf(addr);
                return TRUE;
            }
            else if (current_exec_index == 3) {
                unsigned char addr = get_cpu_data_buf();
                load_addr((unsigned char)(addr + 1), 2);
                return TRUE;
            }
            else if (current_exec_index == 4) {
                store_memory(get_cpu_addr_buf(), data);
                goto addr_mode_done;
            }
            break;

        case ADDR_MODE_INDIR_INDEX:
            //Zero Page Indirect Indexed with Y: (zp),y takes 5 cycles.
            if (current_exec_index == 0) {
                load_memory(cpu_reg.pc);
                cpu_reg.pc++;
                return TRUE;
            }
            else if (current_exec_index == 1) {
                unsigned char addr = get_cpu_data_buf();
                load_addr(addr, 1);
                //save addr in the temporary buffer.
                set_cpu_data_buf(addr);
                return TRUE;
            }
            else if (current_exec_index == 2) {
                unsigned char addr = get_cpu_data_buf();
                load_addr((unsigned char)(addr + 1), 2);
                return TRUE;
            }
            else if (current_exec_index == 3) {
                unsigned short addr = get_cpu_addr_buf();
                addr += cpu_reg.y;
                set_cpu_addr_buf(addr);
                return TRUE;
            }
            else if (current_exec_index == 4) {
                unsigned short addr = get_cpu_addr_buf();
                store_memory(addr, data);
                goto addr_mode_done;
            }
            break;

        default:
            return FALSE;
    }
    return FALSE;

addr_mode_done:
    *done = TRUE;
    return TRUE;
}

/*
 * for inc/dec/shift operation
 * */
static int memory_to_memory(int *do_operation, int *done) {
    BUS_READY_CHECK()

    switch (current_inst->addr_mode) {
        case ADDR_MODE_ACC:
        case ADDR_MODE_IMP:
        case ADDR_MODE_IMM:
        case ADDR_MODE_REL:
        case ADDR_MODE_ZP_Y:
        case ADDR_MODE_ABS_Y:
        case ADDR_MODE_INDEX_INDIR:
        case ADDR_MODE_INDIR_INDEX:
            //not supported.
            return FALSE;

        case ADDR_MODE_ZP:
            //zp takes 4 cycles.
            if (current_exec_index == 0) {
                load_memory(cpu_reg.pc);
                cpu_reg.pc++;
                return TRUE;
            }
            else if (current_exec_index == 1) {
                unsigned short zp = get_cpu_data_buf();
                load_memory(zp);
                set_cpu_addr_buf(zp);
                return TRUE;
            }
            else if (current_exec_index == 2) {
                //arithmetic operation here!!
                //result is set in cpu_data_buf
                *do_operation = TRUE;
                return TRUE;
            }
            else if (current_exec_index == 3) {
                unsigned short zp = get_cpu_addr_buf();
                unsigned short data = get_cpu_data_buf();
                store_memory(zp, data);
                goto mm_done;
            }
            break;

        case ADDR_MODE_ZP_X:
            //zp indexed with x takes 5 cycles.
            if (current_exec_index == 0) {
                load_memory(cpu_reg.pc);
                cpu_reg.pc++;
                return TRUE;
            }
            else if (current_exec_index == 1) {
                unsigned char imm = get_cpu_data_buf();
                set_cpu_data_buf(imm + cpu_reg.x);
                return TRUE;
            }
            else if (current_exec_index == 2) {
                unsigned char zp = get_cpu_data_buf();
                load_memory(zp);
                set_cpu_addr_buf(zp);
                return TRUE;
            }
            else if (current_exec_index == 3) {
                //arithmetic operation here!!
                //result is set in cpu_data_buf
                *do_operation = TRUE;
                return TRUE;
            }
            else if (current_exec_index == 4) {
                unsigned char zp = get_cpu_addr_buf();
                unsigned short data = get_cpu_data_buf();
                store_memory(zp, data);
                goto mm_done;
            }
            break;

        case ADDR_MODE_ABS:
            //takes 5 cycles.
            if (current_exec_index == 0) {
                load_addr(cpu_reg.pc, 1);
                cpu_reg.pc++;
                return TRUE;
            }
            else if (current_exec_index == 1) {
                load_addr(cpu_reg.pc, 2);
                cpu_reg.pc++;
                return TRUE;
            }
            else if (current_exec_index == 2) {
                unsigned short addr = get_cpu_addr_buf();
                load_memory(addr);
                return TRUE;
            }
            else if (current_exec_index == 3) {
                //arithmetic operation here!!
                //result is set in cpu_data_buf
                *do_operation = TRUE;
                return TRUE;
            }
            else if (current_exec_index == 4) {
                unsigned short addr = get_cpu_addr_buf();
                unsigned short data = get_cpu_data_buf();
                store_memory(addr, data);
                goto mm_done;
            }
            break;

        case ADDR_MODE_ABS_X:
            //abs indexed with x takes 6 cycles.
            if (current_exec_index == 0) {
                load_addr(cpu_reg.pc, 1);
                cpu_reg.pc++;
                return TRUE;
            }
            else if (current_exec_index == 1) {
                load_addr(cpu_reg.pc, 2);
                cpu_reg.pc++;
                return TRUE;
            }
            else if (current_exec_index == 2) {
                unsigned short abs = get_cpu_addr_buf();
                set_cpu_addr_buf(abs + cpu_reg.x);
                return TRUE;
            }
            else if (current_exec_index == 3) {
                unsigned short addr = get_cpu_addr_buf();
                load_memory(addr);
                return TRUE;
            }
            else if (current_exec_index == 4) {
                //arithmetic operation here!!
                //result is set in cpu_data_buf
                *do_operation = TRUE;
                return TRUE;
            }
            else if (current_exec_index == 5) {
                unsigned short addr = get_cpu_addr_buf();
                unsigned short data = get_cpu_data_buf();
                store_memory(addr, data);
                goto mm_done;
            }
            break;

        default:
            return FALSE;
    }
    return FALSE;

mm_done:
    *done = TRUE;
    return TRUE;
}


/*-------------   flag operation..  ---------------------*/

static void set_zero(unsigned char data) {
    if (data == 0)
        cpu_reg.status.zero = 1;
    else
        cpu_reg.status.zero = 0;
}

static void set_negative(unsigned char data) {
    if (data & N_BIT)
        cpu_reg.status.negative = 1;
    else
        cpu_reg.status.negative = 0;
}

static void set_CMP_carry(unsigned char data, unsigned char cmp) {
    if (data >= cmp)
        cpu_reg.status.carry = 1;
    else
        cpu_reg.status.carry = 0;
}

static void set_BIT_overflow(unsigned char data) {
    if (data & V_BIT)
        cpu_reg.status.overflow = 1;
    else
        cpu_reg.status.overflow = 0;
}

/*
 * c Set if unsigned overflow; cleared if valid unsigned result.
 * */
static void set_ADD_carry(unsigned char d1, unsigned char d2, unsigned char d3) {
    unsigned short d1_short = d1;
    unsigned short d2_short = d2;
    unsigned short d3_short = d3;
    if (d1_short + d2_short + d3_short > 0xff)
        cpu_reg.status.carry = 1;
    else
        cpu_reg.status.carry = 0;
}

/*
 * v Set if signed overflow; cleared if valid signed result.
 * */
static void set_ADD_overflow(char d1, char d2, char d3) {
    short d1_short = d1;
    short d2_short = d2;
    short d3_short = d3;
    if (d1_short + d2_short + d3_short > +127 || d1_short + d2_short + d3_short < -128 )
        cpu_reg.status.overflow = 1;
    else
        cpu_reg.status.overflow = 0;
}

/*
 * c Set if unsigned borrow not required; cleared if unsigned borrow.
 * */
static void set_SUB_carry(unsigned char d1, unsigned char d2, unsigned char d3) {
    unsigned short d1_short = d1;
    unsigned short d2_short = d2;
    unsigned short d3_short = d3;

    if (d1_short >=  d2_short + d3_short)
        cpu_reg.status.carry = 1;
    else
        cpu_reg.status.carry = 0;
}

/*
 * v Set if signed borrow required; cleared if no signed borrow.
 * */
static void set_SUB_overflow(char d1, char d2, char d3) {
    short d1_short = d1;
    short d2_short = d2;
    short d3_short = d3;
    if ((d1_short - d2_short - d3_short) > +127 || (d1_short - d2_short - d3_short) < -128 ){
        cpu_reg.status.overflow = 1;
    }
    else {
        cpu_reg.status.overflow = 0;
    }
}

/*-------------   stack operation..  ---------------------*/
//stack operation takes two cycles.
static int push(unsigned char data) {
    BUS_READY_CHECK()

    store_memory(STACK_BASE + cpu_reg.sp, data);
    cpu_reg.sp--;
    return TRUE;
}

static int pop(void) {
    BUS_READY_CHECK()

    cpu_reg.sp++;
    load_memory(STACK_BASE + cpu_reg.sp);
    return TRUE;
}

/*---------- instruction implementations.   -----------------*/

/*
 * Add Memory to Accumulator with Carry: ADC
 * A + M + C -> A
 * Flags: N, V, Z, C
 * */
int func_ADC(void) {
    int done = FALSE;
    int ret;
    unsigned char data;
    unsigned char old_carry;

    ret = load_addr_mode(&done);
    if (!ret)
        return FALSE;

    if (!done) 
        return TRUE;

    data = get_cpu_data_buf();
    old_carry = cpu_reg.status.carry;
    //signed, unsigned overflow check.
    set_ADD_carry(cpu_reg.acc, cpu_reg.status.carry, data);
    set_ADD_overflow(cpu_reg.acc, cpu_reg.status.carry, data);
    //add data with carry to accumurator.
    cpu_reg.acc += data + old_carry;

    // N/Z flags set.
    set_negative(cpu_reg.acc);
    set_zero(cpu_reg.acc);

    exec_done = TRUE;
    return TRUE;
}

/*
 * AND Memory with Accumulator: AND
 * A & M -> A
 * Flags: N, Z
 * */
int func_AND(void) {
    int done = FALSE;
    int ret;
    unsigned char data;

    ret = load_addr_mode(&done);
    if (!ret)
        return FALSE;

    if (!done) 
        return TRUE;

    data = get_cpu_data_buf();
    cpu_reg.acc = cpu_reg.acc & data;
    //N/Z flags set.
    set_negative(cpu_reg.acc);
    set_zero(cpu_reg.acc);

    exec_done = TRUE;
    return TRUE;
}

/*
 * Arithmetic Shift Left One Bit: ASL
 * C <- 7 6 5 4 3 2 1 0 <- 0
 * Flags: N, Z, C
 * */
int func_ASL(void) {
    int done = FALSE;
    int operation = FALSE;
    unsigned char data;
    int ret;

    if (current_inst->addr_mode == ADDR_MODE_ACC) {
        unsigned char op_data = cpu_reg.acc;
        //set carry flag from the pre-opration value.
        cpu_reg.status.carry = ((op_data & 0x80) != 0);
        cpu_reg.acc = (op_data << 1);
        set_negative(cpu_reg.acc);
        set_zero(cpu_reg.acc);
        goto acc_done;
    }
    else {
        ret = memory_to_memory(&operation, &done);
        if (!ret)
            return FALSE;

        if (operation) {
            unsigned char op_data = get_cpu_data_buf();
            //set carry flag from the pre-opration value.
            cpu_reg.status.carry = ((op_data & 0x80) != 0);
            op_data = (op_data << 1);
            set_cpu_data_buf(op_data);
            return TRUE;
        }
    }

    if (!done) 
        return TRUE;

    // N/Z flags set.
    data = get_cpu_data_buf();
    set_negative(data);
    set_zero(data);

acc_done:
    exec_done = TRUE;
    return TRUE;
}

static int branch(unsigned char condition) {

    if (current_exec_index == 0) {
        int done = FALSE;
        int ret;
        ret = load_addr_mode(&done);
        if (!ret)
            return FALSE;

        if (!condition) {
            exec_done = TRUE;
        }
        return TRUE;
    }
    else if (current_exec_index == 1) {
        //case branch
        char rel = get_cpu_data_buf();
        unsigned short addr = cpu_reg.pc;
        unsigned short br_addr = cpu_reg.pc + rel;

        if ((addr >> 8) == (br_addr >> 8)) {
            //in page branch.
            cpu_reg.pc = br_addr;
            exec_done = TRUE;
        }
        else {
            set_cpu_addr_buf(br_addr);
        }
        return TRUE;
    }
    else if (current_exec_index == 2) {
        //cross page branch.
        unsigned short br_addr = get_cpu_addr_buf();
        cpu_reg.pc = br_addr;
        exec_done = TRUE;
        return TRUE;
    }

    return FALSE;
}

/*
 * Branch on Carry Clear: BCC
 * Branch if C = 0
 * Flags: none
 * */
int func_BCC(void) {
    return branch(cpu_reg.status.carry == 0);
}

/*
 *  Branch on Carry Set: BCS
 *  Branch if C = 1
 *  Flags: none
 * */
int func_BCS(void) {
    return branch(cpu_reg.status.carry == 1);
}

/*
 *  Branch on Result Zero: BEQ
 *  Branch if Z = 1
 *  Flags: none
 * */
int func_BEQ(void) {
    return branch(cpu_reg.status.zero == 1);
}

/*
 * Test Bits in Memory with Accumulator: BIT
 * A & M
 * Flags: N = M7, V = M6, Z
 * */
int func_BIT(void) {
    int done = FALSE;
    int ret;
    unsigned char cmp;

    ret = load_addr_mode(&done);
    if (!ret)
        return FALSE;

    if (!done) 
        return TRUE;

    cmp = get_cpu_data_buf();
    //cmp N/V/Z flags set.
    /*
     * The BIT instruction really performs two distinct operations.
     * First, it directly transfers the highest and next to highest 
     * bits of the memory operand (that is, seven and six if m
     * = 1, or fifteen and fourteen if m = 0) to the n and v flags. 
     * It does this without modifying the value in the
     * accumulator, making it useful for testing the sign of a value 
     * in memory without loading it into one of the
     * registers. An exception to this is the case where the immediate 
     * addressing mode is used with the BIT
     * instruction: since it serves no purpose to test the bits of a 
     * constant value, the n and v flags are left unchanged in
     * this one case.
     * BITs second operation is to logically AND the value of the 
     * memory operand with the value in the
     * accumulator, conditioning the z flag in the status register to 
     * reflect whether or not the result of the ANDing was
     * zero or not, but without storing the result in the accumulator
     * */
    //6502 core supports zp and abs only
    set_negative(cmp);
    set_BIT_overflow(cmp);
    set_zero(cpu_reg.acc & cmp);

    exec_done = TRUE;
    return TRUE;
}

/*
 *  Branch on Result Minus: BMI
 *  Branch if N = 1
 *  Flags: none
 * */
int func_BMI(void) {
    return branch(cpu_reg.status.negative == 1);
}

/*
 * Branch on Result not Zero: BNE
 * Branch if Z = 0
 * Flags: none
 * */
int func_BNE(void) {
    return branch(cpu_reg.status.zero == 0);
}

/*
 * Branch on Result Plus: BPL
 * Branch if N = 0
 * Flags: none
 * */
int func_BPL(void) {
    return branch(cpu_reg.status.negative == 0);
}

int func_BRK(void) {
    return FALSE;
}

/*
 * Branch on Overflow Clear: BVC
 * Branch if V = 0
 * Flags: none
 * */
int func_BVC(void) {
    return branch(cpu_reg.status.overflow == 0);
}

/*
 * Branch on Overflow Set: BVS
 * Branch if V = 1
 * Flags: none
 * */
int func_BVS(void) {
    return branch(cpu_reg.status.overflow == 1);
}

/*
 * Clear Carry Flag: CLC
 * 0 -> C
 * Flags: C = 0
 * */
int func_CLC(void) {
    cpu_reg.status.carry = 0;
    exec_done = TRUE;
    return TRUE;
}

/*
 * Clear Decimal Mode: CLD
 * 0 -> D
 * Flags: D = 0
 *
 * NOTE: Decimal mode is not implemented on NES core.
 * */
int func_CLD(void) {
    cpu_reg.status.decimal = 0;
    exec_done = TRUE;
    return TRUE;
}

/*
 * Clear Interrupt Disable Status: CLI
 * 0 -> I
 * Flags: I = 0
 * */
int func_CLI(void) {
    cpu_reg.status.irq_disable = 0;
    exec_done = TRUE;
    return TRUE;
}

/*
 * Clear Overflow Flag: CLV
 * 0 -> V
 * Flags: V = 0
 * */
int func_CLV(void) {
    cpu_reg.status.overflow = 0;
    exec_done = TRUE;
    return TRUE;
}

/*
 * Compare Memory and Accumulator: CMP
 * A - M
 * Flags: N, Z, C
 * */
int func_CMP(void) {
    int done = FALSE;
    int ret;
    unsigned char cmp;

    ret = load_addr_mode(&done);
    if (!ret)
        return FALSE;

    if (!done) 
        return TRUE;

    cmp = get_cpu_data_buf();
    //cmp C/N/Z flags set.
    set_CMP_carry(cpu_reg.acc, cmp);
    set_negative(cpu_reg.acc - cmp);
    set_zero(cpu_reg.acc - cmp);

    exec_done = TRUE;
    return TRUE;
}

/*
 * Compare Memory and Index X: CPX
 * X - M
 * Flags: N, Z, C
 * */
int func_CPX(void) {
    int done = FALSE;
    int ret;
    unsigned char cmp;

    ret = load_addr_mode(&done);
    if (!ret)
        return FALSE;

    if (!done) 
        return TRUE;

    cmp = get_cpu_data_buf();
    //cmp C/N/Z flags set.
    set_CMP_carry(cpu_reg.x, cmp);
    set_negative(cpu_reg.x - cmp);
    set_zero(cpu_reg.x - cmp);

    exec_done = TRUE;
    return TRUE;
}

/*
 * Compare Memory with Index Y: CPY
 * Y - M
 * Flags: N, Z, C
 * */
int func_CPY(void) {
    int done = FALSE;
    int ret;
    unsigned char cmp;

    ret = load_addr_mode(&done);
    if (!ret)
        return FALSE;

    if (!done) 
        return TRUE;

    cmp = get_cpu_data_buf();
    //cmp C/N/Z flags set.

/*
 * Subtract the data located at the effective address specified 
 * by the operand from the contents of the Y
 * register, setting the carry, zero, and negative flags based 
 * on the result, but without altering the contents of either
 * the memory location or the register. The comparison is of 
 * unsigned values only (expect for signed comparison
 * for equality).
 * */
    set_CMP_carry(cpu_reg.y, cmp);
    set_negative(cpu_reg.y - cmp);
    set_zero(cpu_reg.y - cmp);

    exec_done = TRUE;
    return TRUE;
}

/*
 * Decrement Memory by One: DEC
 * M - 1 -> M
 * Flags: N, Z
 * */
int func_DEC(void) {
    int done = FALSE;
    int operation = FALSE;
    unsigned char data;
    int ret;

    ret = memory_to_memory(&operation, &done);
    if (!ret)
        return FALSE;

    if (operation) {
        unsigned char op_data = get_cpu_data_buf();
        op_data--;
        set_cpu_data_buf(op_data);
        return TRUE;
    }

    if (!done) 
        return TRUE;

    // N/Z flags set.
    data = get_cpu_data_buf();
    set_negative(data);
    set_zero(data);

    exec_done = TRUE;
    return TRUE;
}

/*
 * Decrement Index X by One: DEX
 * X - 1 -> X
 * Flags: N, Z
 * */
int func_DEX(void) {
    cpu_reg.x--;

    //ldx N/Z flags set.
    set_negative(cpu_reg.x);
    set_zero(cpu_reg.x);

    exec_done = TRUE;
    return TRUE;
}

/*
 * Decrement Index Y by One: DEY
 * Y - 1 -> Y
 * Flags: N, Z
 * */
int func_DEY(void) {
    cpu_reg.y--;

    //ldx N/Z flags set.
    set_negative(cpu_reg.y);
    set_zero(cpu_reg.y);

    exec_done = TRUE;
    return TRUE;
}

/*
 * Exclusive-OR Memory with Accumulator: EOR
 * A ^ M -> A
 * Flags: N, Z
 * */
int func_EOR(void) {
    int done = FALSE;
    int ret;
    unsigned char data;

    ret = load_addr_mode(&done);
    if (!ret)
        return FALSE;

    if (!done) 
        return TRUE;

    data = get_cpu_data_buf();
    cpu_reg.acc = cpu_reg.acc ^ data;
    //N/Z flags set.
    set_negative(cpu_reg.acc);
    set_zero(cpu_reg.acc);

    exec_done = TRUE;
    return TRUE;
}

/*
 * Increment Memory by One: INC
 * M + 1 -> M
 * Flags: N, Z
 * */
int func_INC(void) {
    int done = FALSE;
    int operation = FALSE;
    unsigned char data;
    int ret;

    ret = memory_to_memory(&operation, &done);
    if (!ret)
        return FALSE;

    if (operation) {
        unsigned char op_data = get_cpu_data_buf();
        op_data++;
        set_cpu_data_buf(op_data);
        return TRUE;
    }

    if (!done) 
        return TRUE;

    // N/Z flags set.
    data = get_cpu_data_buf();
    set_negative(data);
    set_zero(data);

    exec_done = TRUE;
    return TRUE;
}

/*
 * Increment Index X by One: INX
 * X + 1 -> X
 * Flags: N, Z
 * */
int func_INX(void) {
    cpu_reg.x++;

    //ldx N/Z flags set.
    set_negative(cpu_reg.x);
    set_zero(cpu_reg.x);

    exec_done = TRUE;
    return TRUE;
}

int func_INY(void) {
    cpu_reg.y++;

    //ldx N/Z flags set.
    set_negative(cpu_reg.y);
    set_zero(cpu_reg.y);

    exec_done = TRUE;
    return TRUE;
}

static int jmp(int cycle, int *done) {
    BUS_READY_CHECK()

    switch (current_inst->addr_mode) {
        case ADDR_MODE_ABS:
            //takes 2 cycles.
            if (cycle == 0) {
                load_addr(cpu_reg.pc, 1);
                cpu_reg.pc++;
                return TRUE;
            }
            else if (cycle == 1) {
                load_addr(cpu_reg.pc, 2);
                cpu_reg.pc++;
                *done = TRUE;
                return TRUE;
            }
            break;

        case ADDR_MODE_IND:
            //takes 4 cycles.
            if (cycle == 0) {
                load_addr(cpu_reg.pc, 1);
                cpu_reg.pc++;
                return TRUE;
            }
            else if (cycle == 1) {
                load_addr(cpu_reg.pc, 2);
                cpu_reg.pc++;
                return TRUE;
            }
            else if (cycle == 2) {
                load_memory(get_cpu_addr_buf());
                return TRUE;
            }
            else if (cycle == 3) {
                unsigned char low, hi;
                unsigned short addr;
                low = get_cpu_data_buf();
                hi = load_memory(get_cpu_addr_buf() + 1);
                addr = (hi << 8) | low;
                set_cpu_addr_buf(addr);
                *done = TRUE;
                return TRUE;
            }
            break;

        default:
            return FALSE;
    }
    return FALSE;
}

/*
 * Jump to New Location: JMP
 * Jump to new location
 * Flags: none
 * */
int func_JMP(void) {
    int done = FALSE;
    int ret;

    ret = jmp(current_exec_index, &done);
    if (!ret)
        return FALSE;

    if (!done) 
        return TRUE;

    cpu_reg.pc = get_cpu_addr_buf();

    exec_done = TRUE;
    return TRUE;
}

/*
 * Jump to New Location Saving Return Address: JSR
 * Jump to Subroutine
 * Flags: none
 * */
int func_JSR(void) {
    int done = FALSE;

    //cycle 1
    if (current_exec_index == 0) {
        //save return addr(-1) hi.
        //pc + 1 => jsr abslo abshi - 1
        return push((cpu_reg.pc + 1) >> 8);
    }
    //cycle 2
    else if (current_exec_index == 1) {
        //save return addr(-1) low.
        return push(cpu_reg.pc + 1);
    }
    //cycle 3,4
    else if (current_exec_index < 4) {
        return jmp(current_exec_index - 2, &done);
    }
    //cycle 5
    else if (current_exec_index == 4) {
        cpu_reg.pc = get_cpu_addr_buf();
        exec_done = TRUE;
        return TRUE;
    }
    return FALSE;
}

/*
 * Load Accumulator with Memory: LDA
 * M -> A
 * Flags: N, Z
 * */
int func_LDA(void) {
    int done = FALSE;
    int ret;

    ret = load_addr_mode(&done);
    if (!ret)
        return FALSE;

    if (!done) 
        return TRUE;

    cpu_reg.acc = get_cpu_data_buf();
    //ldx N/Z flags set.
    set_negative(cpu_reg.acc);
    set_zero(cpu_reg.acc);

    exec_done = TRUE;
    return TRUE;
}

/*
 * Load Index X with Memory: LDX
 * M -> X
 * Flags: N, Z
 * */
int func_LDX(void) {
    int done = FALSE;
    int ret;

    ret = load_addr_mode(&done);
    if (!ret)
        return FALSE;

    if (!done) 
        return TRUE;

    cpu_reg.x = get_cpu_data_buf();
    //ldx N/Z flags set.
    set_negative(cpu_reg.x);
    set_zero(cpu_reg.x);

    exec_done = TRUE;
    return TRUE;
}

/*
 * Load Index Y with Memory: LDY
 * M -> Y
 * Flags: N, Z
 * */
int func_LDY(void) {
    int done = FALSE;
    int ret;

    ret = load_addr_mode(&done);
    if (!ret)
        return FALSE;

    if (!done) 
        return TRUE;

    cpu_reg.y = get_cpu_data_buf();
    //ldx N/Z flags set.
    set_negative(cpu_reg.y);
    set_zero(cpu_reg.y);

    exec_done = TRUE;
    return TRUE;
}

/*
 * Logical Shift Right One Bit: LSR
 * 0 -> 7 6 5 4 3 2 1 0 -> C
 * Flags: N, Z, C
 * */
int func_LSR(void) {
    int done = FALSE;
    int operation = FALSE;
    unsigned char data;
    int ret;

    if (current_inst->addr_mode == ADDR_MODE_ACC) {
        unsigned char op_data = cpu_reg.acc;
        //set carry flag from the pre-opration value.
        cpu_reg.status.carry = (op_data & 0x01);
        cpu_reg.acc = (op_data >> 1);
        set_negative(cpu_reg.acc);
        set_zero(cpu_reg.acc);
        goto acc_done;
    }
    else {
        ret = memory_to_memory(&operation, &done);
        if (!ret)
            return FALSE;

        if (operation) {
            unsigned char op_data = get_cpu_data_buf();
            //set carry flag from the pre-opration value.
            cpu_reg.status.carry = (op_data & 0x01);
            op_data = (op_data >> 1);
            set_cpu_data_buf(op_data);
            return TRUE;
        }
    }

    if (!done) 
        return TRUE;

    // N/Z flags set.
    data = get_cpu_data_buf();
    set_negative(data);
    set_zero(data);

acc_done:
    exec_done = TRUE;
    return TRUE;
}

/*
 * No Operation: NOP
 * No Operation
 * Flags: none
 * */
int func_NOP(void) {
    //do nothing.
    exec_done = TRUE;
    return TRUE;
}

/*
 * OR Memory with Accumulator: ORA
 * A | M -> A
 * Flags: N, Z
 * */
int func_ORA(void) {
    int done = FALSE;
    int ret;
    unsigned char data;

    ret = load_addr_mode(&done);
    if (!ret)
        return FALSE;

    if (!done) 
        return TRUE;

    data = get_cpu_data_buf();
    cpu_reg.acc = cpu_reg.acc | data;
    //N/Z flags set.
    set_negative(cpu_reg.acc);
    set_zero(cpu_reg.acc);

    exec_done = TRUE;
    return TRUE;
}

/*
 * push takes 2 cycles.
 * */
static int push_op(unsigned char data, int *done) {
    //cycle 1
    if (current_exec_index == 0) {
        return push(data);
    }
    //cycle 2
    else if (current_exec_index == 1) {
        //cycle 2 doesn't do enything.
        *done = TRUE;
        return TRUE;
    }
    return FALSE;
}

/*
 * Push Accumulator on Stack: PHA
 * A -> S
 * Flags: none
 * */
int func_PHA(void) {
    int ret;
    int done = FALSE;
    ret = push_op(cpu_reg.acc, &done);
    exec_done = done;
    return ret;
}

/*
 * Push Processor Status on Stack: PHP
 * P -> S
 * The processor status is stored as a single byte with the 
 * following flags bits from high to low: NV-BDIZC.
 *
 * Flags: none
 * */
int func_PHP(void) {
    int ret;
    int done = FALSE;
    unsigned char st;
    memcpy(&st, &cpu_reg.status, sizeof(struct status_reg));
    ret = push_op(st, &done);
    exec_done = done;
    return ret;
}

/*
 * pull takes 3 cycles.
 * */
static int pull_op(int *done) {
    //cycle 1
    if (current_exec_index == 0) {
        return pop();
    }
    //cycle 2
    else if (current_exec_index == 1) {
        //cycle 2 doesn't do enything.
        return TRUE;
    }
    //cycle 3
    else if (current_exec_index == 2) {
        //cycle 3 caller must xfer data from cpu data buf
        *done = TRUE;
        return TRUE;
    }
    return FALSE;
}

/*
 * Pull Accumulator from Stack: PLA
 * S -> A
 * Flags: N, Z
 * */
int func_PLA(void) {
    int ret;
    int done = FALSE;
    ret = pull_op(&done);
    if (done) {
        cpu_reg.acc = get_cpu_data_buf();
        set_negative(cpu_reg.acc);
        set_zero(cpu_reg.acc);
    }
    exec_done = done;
    return ret;
}

/*
 * Pull Processor Status from Stack: PLP
 * S -> P
 * Setting the processor status from the stack is the only way to clear the B (Break) flag.
 * Flags: all
 * */
int func_PLP(void) {
    int ret;
    int done = FALSE;
    ret = pull_op(&done);
    if (done) {
        unsigned char st;
        st = get_cpu_data_buf();
        memcpy(&cpu_reg.status, &st, sizeof(struct status_reg));
    }
    /* researved bit always 1*/
    cpu_reg.status.researved = 1;
    /* decimal always 0*/
    cpu_reg.status.decimal = 0;
    exec_done = done;
    return ret;
}

/*
 * Rotate Left One Bit: ROL
 * C <- 7 6 5 4 3 2 1 0 <- C
 * Flags: N, Z, C
 * */
int func_ROL(void) {
    int done = FALSE;
    int operation = FALSE;
    unsigned char data;
    int ret;

    if (current_inst->addr_mode == ADDR_MODE_ACC) {
        unsigned char op_data = cpu_reg.acc;
        unsigned char old_carry = cpu_reg.status.carry;

        //set carry flag from the pre-opration value.
        cpu_reg.status.carry = ((op_data & 0x80) != 0);
        cpu_reg.acc = (op_data << 1);
        if (old_carry)
            cpu_reg.acc |= 0x01;
        set_negative(cpu_reg.acc);
        set_zero(cpu_reg.acc);
        goto acc_done;
    }
    else {
        ret = memory_to_memory(&operation, &done);
        if (!ret)
            return FALSE;

        if (operation) {
            unsigned char op_data = get_cpu_data_buf();
            unsigned char old_carry = cpu_reg.status.carry;

            //set carry flag from the pre-opration value.
            cpu_reg.status.carry = ((op_data & 0x80) != 0);
            op_data = (op_data << 1);
            if (old_carry)
                op_data |= 0x01;
            set_cpu_data_buf(op_data);
            return TRUE;
        }
    }

    if (!done) 
        return TRUE;

    // N/Z flags set.
    data = get_cpu_data_buf();
    set_negative(data);
    set_zero(data);

acc_done:
    exec_done = TRUE;
    return TRUE;
}

/*
 * Rotate Right One Bit: ROR
 * C -> 7 6 5 4 3 2 1 0 -> C
 * Flags: N, Z, C
 * */
int func_ROR(void) {
    int done = FALSE;
    int operation = FALSE;
    unsigned char data;
    int ret;

    if (current_inst->addr_mode == ADDR_MODE_ACC) {
        unsigned char op_data = cpu_reg.acc;
        unsigned char old_carry = cpu_reg.status.carry;

        //set carry flag from the pre-opration value.
        cpu_reg.status.carry = (op_data & 0x01);
        cpu_reg.acc = (op_data >> 1);
        if (old_carry)
            cpu_reg.acc |= 0x80;
        set_negative(cpu_reg.acc);
        set_zero(cpu_reg.acc);
        goto acc_done;
    }
    else {
        ret = memory_to_memory(&operation, &done);
        if (!ret)
            return FALSE;

        if (operation) {
            unsigned char op_data = get_cpu_data_buf();
            unsigned char old_carry = cpu_reg.status.carry;

            //set carry flag from the pre-opration value.
            cpu_reg.status.carry = (op_data & 0x01);
            op_data = (op_data >> 1);
            if (old_carry)
                op_data |= 0x80;
            set_cpu_data_buf(op_data);
            return TRUE;
        }
    }

    if (!done) 
        return TRUE;

    // N/Z flags set.
    data = get_cpu_data_buf();
    set_negative(data);
    set_zero(data);

acc_done:
    exec_done = TRUE;
    return TRUE;
}

/*
 * Return from Interrupt: RTI
 * Return from Interrupt
 * Flags: all
 * */
int func_RTI(void) {
    //cycle 1
    if (current_exec_index == 0) {
        //pop statu reg.
        return pop();
    }
    //cycle 2 
    else if (current_exec_index == 1) {
        unsigned char data;
        //set status reg
        data = get_cpu_data_buf();
        memcpy(&cpu_reg.status, &data, sizeof(data));
        //pop return addr low.
        return pop();
    }
    //cycle 3
    else if (current_exec_index == 2) {
        //set return addr low.
        set_cpu_addr_buf(get_cpu_data_buf());
        //pop return addr hi.
        return pop();
    }
    //cycle 4
    else if (current_exec_index == 3) {
        unsigned char hi, lo;
        unsigned short addr;

        //set return addr hi
        lo = get_cpu_addr_buf();
        hi = get_cpu_data_buf();
        addr = (hi << 8) | lo;
        set_cpu_addr_buf(addr);
        return TRUE;
    }
    //cycle 5
    else if (current_exec_index == 4) {
        //set pc = addr
        cpu_reg.pc = get_cpu_addr_buf();
        exec_done = TRUE;
        return TRUE;
    }
    return FALSE;
}

/*
 * Return from Subroutine: RTS
 * Return from Subroutine
 * Flags: none
 * */
int func_RTS(void) {

    //cycle 1
    if (current_exec_index == 0) {
        //pop return addr low.
        return pop();
    }
    //cycle 2 
    else if (current_exec_index == 1) {
        //set return addr low.
        set_cpu_addr_buf(get_cpu_data_buf());
        return TRUE;
    }
    //cycle 3
    else if (current_exec_index == 2) {
        //pop return addr hi.
        return pop();
    }
    //cycle 4
    else if (current_exec_index == 3) {
        unsigned char hi, lo;
        unsigned short addr;

        //set return addr hi
        lo = get_cpu_addr_buf();
        hi = get_cpu_data_buf();
        addr = (hi << 8) | lo;
        set_cpu_addr_buf(addr);
        return TRUE;
    }
    //cycle 5
    else if (current_exec_index == 4) {
        //set pc = addr + 1
        cpu_reg.pc = get_cpu_addr_buf() + 1;
        exec_done = TRUE;
        return TRUE;
    }
    return FALSE;
}

/*
 * Subtract Memory from Accumulator with Borrow: SBC
 * A - M - ~C -> A
 * Flags: N, V, Z, C
 * */
int func_SBC(void) {
    int done = FALSE;
    int ret;
    unsigned char data;
    unsigned char c_comp;

    ret = load_addr_mode(&done);
    if (!ret)
        return FALSE;

    if (!done) 
        return TRUE;

    data = get_cpu_data_buf();
    c_comp = (cpu_reg.status.carry == 0 ? 1 : 0);

    //signed, unsigned overflow check.
    set_SUB_carry(cpu_reg.acc, data, c_comp);
    set_SUB_overflow(cpu_reg.acc, data, c_comp);

    //subtract data with carry to accumurator.
    cpu_reg.acc = cpu_reg.acc - data - c_comp;

    // N/Z flags set.
    set_negative(cpu_reg.acc);
    set_zero(cpu_reg.acc);

    exec_done = TRUE;
    return TRUE;
}

/*
 * Set Carry Flag: SEC
 * 1 -> C
 * Flags: C = 1
 * */
int func_SEC(void) {
    cpu_reg.status.carry = 1;
    exec_done = TRUE;
    return TRUE;
}

/*
 * Set Decimal Mode: SED
 * 1 -> D
 * Flags: D = 1
 *
 * NOTE: decimal mode is not supported on NES core
 * */
int func_SED(void) {
    cpu_reg.status.decimal = 1;
    fprintf(stderr, "decimal mode is not supported!!!\n");
    return FALSE;
}

/*
 * set interrupt disable.
 * */
int func_SEI(void) {
    cpu_reg.status.irq_disable = 1;
    exec_done = TRUE;
    return TRUE;
}

/*
 * Store Accumulator in Memory: STA
 * A -> M
 * Flags: none
 * */
int func_STA(void) {
    int done = FALSE;
    int ret;

    ret = store_addr_mode(cpu_reg.acc, &done);
    if (!ret)
        return FALSE;

    if (!done) 
        return TRUE;

    exec_done = TRUE;
    return TRUE;
}

/*
 * Store Index X in Memory: STX
 * X -> M
 * Flags: none
 * */
int func_STX(void) {
    int done = FALSE;
    int ret;

    ret = store_addr_mode(cpu_reg.x, &done);
    if (!ret)
        return FALSE;

    if (!done) 
        return TRUE;

    exec_done = TRUE;
    return TRUE;
}

/*
 * Store Index Y in Memory: STY
 * Y -> M
 * Flags: none
 * */
int func_STY(void) {
    int done = FALSE;
    int ret;

    ret = store_addr_mode(cpu_reg.y, &done);
    if (!ret)
        return FALSE;

    if (!done) 
        return TRUE;

    exec_done = TRUE;
    return TRUE;
}

/*
 * Transfer Accumulator to Index X: TAX
 * A -> X
 * Flags: N, Z
 * */
int func_TAX(void) {
    cpu_reg.x = cpu_reg.acc;

    set_negative(cpu_reg.x);
    set_zero(cpu_reg.x);

    exec_done = TRUE;
    return TRUE;
}

/*
 * Transfer Accumulator to Index Y: TAY
 * A -> Y
 * Flags: N, Z
 * */
int func_TAY(void) {
    cpu_reg.y = cpu_reg.acc;

    set_negative(cpu_reg.y);
    set_zero(cpu_reg.y);

    exec_done = TRUE;
    return TRUE;
}

/*
 * Transfer Stack Pointer to Index X: TSX
 * S -> X
 * Flags: N, Z
 * */
int func_TSX(void) {
    cpu_reg.x = cpu_reg.sp;

    set_negative(cpu_reg.x);
    set_zero(cpu_reg.x);

    exec_done = TRUE;
    return TRUE;
}

/*
 * Transfer Index X to Accumulator: TXA
 * X -> A
 * Flags: N, Z
 * */
int func_TXA(void) {
    cpu_reg.acc = cpu_reg.x;

    set_negative(cpu_reg.acc);
    set_zero(cpu_reg.acc);

    exec_done = TRUE;
    return TRUE;
}

/*
 * Transfer Index X to Stack Pointer: TXS
 * X -> S
 * Flags: N, Z
 * */
int func_TXS(void) {
    cpu_reg.sp = cpu_reg.x;

    set_negative(cpu_reg.sp);
    set_zero(cpu_reg.sp);

    exec_done = TRUE;
    return TRUE;
}

/*
 * Transfer Index Y to Accumulator: TYA
 * Y -> A
 * Flags: N, Z
 * */
int func_TYA(void) {
    cpu_reg.acc = cpu_reg.y;

    set_negative(cpu_reg.y);
    set_zero(cpu_reg.y);

    exec_done = TRUE;
    return TRUE;
}

/* ------------------     6502 execution..      ------------------- */

/*
 * decode6502:
 * return execution cycle count
 * */
int decode6502(unsigned char inst) {

    struct opcode_map * omap = &opcode_list[inst];
    if (omap->func == NULL) {
        return FALSE;
    }

    /*dprint("decode inst: %02x > %s, %d cycle, %d len\n", 
            inst, omap->mnemonic, omap->cycle, omap->inst_len);
*/

    current_inst = omap;

    return TRUE;
}

int test_and_set_exec(void) {
    int ret;
    ret = exec_done;
    if (exec_done) {
        exec_done = FALSE;
        current_exec_index = 0;
    }
    return ret;
}

int execute6502(void) {
    int ret;
    ret = current_inst->func();
    if (!ret && !bus_status) {
        //if bus status not ready, deson't do anything.
        return TRUE;
    }
    current_exec_index++;

#ifdef cycle_check
    if (exec_done && (current_inst->cycle - 1 != current_exec_index)) {
        if (current_inst->cycle_aux && (
                    current_inst->addr_mode == ADDR_MODE_ABS_X ||
                    current_inst->addr_mode == ADDR_MODE_ABS_Y ||
                    current_inst->addr_mode == ADDR_MODE_INDIR_INDEX ) && 
                (current_inst->cycle == current_exec_index)) {
            ;
        }
        else if ((current_inst->addr_mode == ADDR_MODE_REL) && 
                ((current_inst->cycle == current_exec_index) ||
                (current_inst->cycle + 1 == current_exec_index ))) {
            ;
        }
        else {
            fprintf(stderr, "instruction cycle check error!!\n");
            return FALSE;
        }
    }
#endif

    return ret;
}

int reset_exec6502(void) {
    if ((bus_status = bus_ready()) != TRUE) { 
        //on reset, if bus is not ready, do nothing.
        return TRUE; 
    } 

    switch (current_exec_index++) {
        case 0:
            //step 1: load intvec low.
            load_addr(RESET_VECTOR, 1);
            return TRUE;
        case 1:
            //step 2: load intvec hi.
            load_addr(RESET_VECTOR + 1, 2);
            return TRUE;
        case 2:
            //step 3: set pc
            cpu_reg.pc = get_cpu_addr_buf();
            //set status flag
            cpu_reg.status.decimal = 0;
            cpu_reg.status.irq_disable = 1;
            intr_done = TRUE;
            return TRUE;
    }
    return FALSE;
}

int reset6502(void) {
    current_exec_index = 0;
    nmi_cnt = 0;
    return reset_exec6502();
}

int nmi6502(void) {
    if ((bus_status = bus_ready()) != TRUE) { 
        //on nmi, if bus is not ready, do nothing.
        return TRUE; 
    } 

    //dprint("nmi...\n");

    //nmi6502 is always called when current instruction execution is done.
    switch (current_exec_index++) {
        case 0:
            //first: push pc hi.
            return push(cpu_reg.pc >> 8);
        case 1:
            //second: push pc low.
            return push(cpu_reg.pc);
        case 2:
            {
                //step 3, push status_reg
                unsigned char stat;
                memcpy(&stat, &cpu_reg.status, sizeof(stat));
                return push(stat);
            }
        case 3:
            //step 4: load intvec low.
            load_addr(NMI_VECTOR, 1);
            return TRUE;
        case 4:
            //step 5: load intvec hi.
            load_addr(NMI_VECTOR + 1, 2);
            return TRUE;
        case 5:
            //step 6: set pc
            cpu_reg.pc = get_cpu_addr_buf();
            //set status flag
            cpu_reg.status.decimal = 0;
            cpu_reg.status.irq_disable = 1;

            //reset cpu counter...
            nmi_cnt++;
            reset_clock_cnt();
            intr_done = TRUE;
            return TRUE;
    }
    return FALSE;
}

int test_and_set_intr(void) {
    int ret;
    ret = intr_done;
    if (ret) {
        current_exec_index = 0;
        intr_done = FALSE;
    }
    return ret;
}

void pc_set(unsigned short addr) {
    cpu_reg.pc = addr;
}

unsigned short pc_get(void) {
    return cpu_reg.pc;
}

void pc_move(int offset) {
    cpu_reg.pc += offset;
}

int init_6502core(void) {
    memset(&cpu_reg, 0, sizeof(struct cpu_6502));
    cpu_reg.status.researved = 1;
    current_inst = NULL;
    current_exec_index = 0;
    exec_done = FALSE;
    intr_done = FALSE;
    return TRUE;
}

/* for debug.c */

void dump_6502(int full) {
    //clock is 64 bit format.
    //upper 8 bit is nmi cnt.
    //lower 56 bit is cpu counter from nmi.
    printf("\nclock: %04x%012lx\n", nmi_cnt,
        (unsigned long)(0x0000ffffffffffffff & get_clock_cnt()));
    if (full) 
        printf("6502 CPU registers:\n");

    printf(" pc:     %04x\n", cpu_reg.pc);
    if (full) {
        printf(" acc:    %02x\n", cpu_reg.acc);
        printf(" x:      %02x\n", cpu_reg.x);
        printf(" y:      %02x\n", cpu_reg.y);
        printf(" sp:     %02x\n", cpu_reg.sp);
        printf(" status:\n");
        printf("  negative:   %d\n", cpu_reg.status.negative);
        printf("  overflow:   %d\n", cpu_reg.status.overflow);
        printf("  break:      %d\n", cpu_reg.status.break_mode);
        printf("  decimal:    %d\n", cpu_reg.status.decimal);
        printf("  irq:        %d\n", cpu_reg.status.irq_disable);
        printf("  zero:       %d\n", cpu_reg.status.zero);
        printf("  carry:      %d\n", cpu_reg.status.carry);
        printf("-------------------\n");
    }
    //printf("data:     %02x\n", cpu_data_buffer);
}


int disas_inst(unsigned short addr) {
    unsigned char inst;
    unsigned char dbg_get_byte(unsigned short addr);
    void disasm(const char* mnemonic, int addr_mode, unsigned short pc, struct cpu_6502* reg);

    inst = dbg_get_byte(addr);
    struct opcode_map * omap = &opcode_list[inst];
    
    disasm(omap->mnemonic, omap->addr_mode, addr, &cpu_reg);
    return omap->inst_len;
}

void report_exec_err(void) {
    fprintf(stderr, "cpu execute instruction failure @0x%04x.\n", pc_get());
    fprintf(stderr, "error instruction: %s, cycle:%d\n", 
            current_inst->mnemonic, current_exec_index - 1);
}

unsigned int get_nmi_cnt(void) {
    return nmi_cnt;
}
