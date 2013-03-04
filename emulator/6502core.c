#include <stdio.h>
#include <string.h>
#include <libio.h>
#include "tools.h"

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

#define ADDR_MODE_ZP        0
#define ADDR_MODE_ZP_X      1
#define ADDR_MODE_ZP_Y      2
#define ADDR_MODE_ABS       3
#define ADDR_MODE_ABS_X     4
#define ADDR_MODE_ABS_Y     5
#define ADDR_MODE_IND       6
#define ADDR_MODE_IMP       7
#define ADDR_MODE_ACC       8
#define ADDR_MODE_IMM       9
#define ADDR_MODE_REL       10
#define ADDR_MODE_INDEX_INDIR       11
#define ADDR_MODE_INDIR_INDEX       12

#define N_BIT   0x80

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
static int current_exec_index;
static int exec_done;

unsigned char load_memory(unsigned short addr);
unsigned short load_addr(unsigned short addr, int cycle);

unsigned char get_cpu_data_buf(void);
void set_cpu_data_buf(unsigned char data);
unsigned short get_cpu_addr_buf(void);
void set_cpu_addr_buf(unsigned short addr);

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



static int addr_mode_exec(int *done) {
    switch (current_inst->addr_mode) {
        case ADDR_MODE_ACC:
        case ADDR_MODE_IMP:
            //accumulator/implied doesn't need memory operation.
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
                unsigned short zp = get_cpu_data_buf();
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
                unsigned short zp = get_cpu_data_buf();
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

                    hi_8 = addr >> 8;
                    addr += cpu_reg.x;
                    added_hi_8 = addr >> 8;

                    if (hi_8 == added_hi_8) {
                        load_memory(addr);
                        goto addr_mode_done;
                    }

                    //load value in the next cycle.
                    set_cpu_data_buf(addr);
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
                    unsigned short addr = get_cpu_data_buf();
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

                    hi_8 = addr >> 8;
                    addr += cpu_reg.y;
                    added_hi_8 = addr >> 8;

                    if (hi_8 == added_hi_8) {
                        load_memory(addr);
                        goto addr_mode_done;
                    }

                    //load value in the next cycle.
                    set_cpu_data_buf(addr);
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
                    unsigned short addr = get_cpu_data_buf();
                    load_memory(addr);
                    goto addr_mode_done;
                }
            }
            break;

        case ADDR_MODE_INDEX_INDIR:
        case ADDR_MODE_INDIR_INDEX:
#warning zp indexed indirect, zp indirect indexed must be reworked!!

        default:
            return FALSE;
    }
    return FALSE;

addr_mode_done:
    *done = TRUE;
    return TRUE;
}

int func_ADC(void) {
    return FALSE;
}

int func_AND(void) {
    return FALSE;
}

int func_ASL(void) {
    return FALSE;
}

int func_BCC(void) {
    return FALSE;
}

int func_BCS(void) {
    return FALSE;
}

int func_BEQ(void) {
    return FALSE;
}

int func_BIT(void) {
    return FALSE;
}

int func_BMI(void) {
    return FALSE;
}

int func_BNE(void) {
    return FALSE;
}

int func_BPL(void) {
    return FALSE;
}

int func_BRK(void) {
    return FALSE;
}

int func_BVC(void) {
    return FALSE;
}

int func_BVS(void) {
    return FALSE;
}

int func_CLC(void) {
    return FALSE;
}

int func_CLD(void) {
    return FALSE;
}

int func_CLI(void) {
    return FALSE;
}

int func_CLV(void) {
    return FALSE;
}

int func_CMP(void) {
    return FALSE;
}

int func_CPX(void) {
    return FALSE;
}

int func_CPY(void) {
    return FALSE;
}

int func_DEC(void) {
    return FALSE;
}

int func_DEX(void) {
    return FALSE;
}

int func_DEY(void) {
    return FALSE;
}

int func_EOR(void) {
    return FALSE;
}

int func_INC(void) {
    return FALSE;
}

int func_INX(void) {
    return FALSE;
}

int func_INY(void) {
    return FALSE;
}

int func_JMP(void) {
    return FALSE;
}

int func_JSR(void) {
    return FALSE;
}

int func_LDA(void) {
    return FALSE;
}

/*
 * Load Index X with Memory
 * */
int func_LDX(void) {
    int done = FALSE;
    int ret;

    ret = addr_mode_exec(&done);
    if (!ret)
        return FALSE;

    if (!done) 
        return TRUE;

    cpu_reg.x = get_cpu_data_buf();
    //ldx N/Z flags set.
    if (cpu_reg.x == 0)
        cpu_reg.status.zero = 1;
    if (cpu_reg.x & N_BIT)
        cpu_reg.status.negative = 1;
    exec_done = TRUE;
    return TRUE;
}

/*
 * Load Index Y with Memory
 * */
int func_LDY(void) {
    int done = FALSE;
    int ret;

    ret = addr_mode_exec(&done);
    if (!ret)
        return FALSE;

    if (!done) 
        return TRUE;

    cpu_reg.y = get_cpu_data_buf();
    //ldx N/Z flags set.
    if (cpu_reg.y == 0)
        cpu_reg.status.zero = 1;
    if (cpu_reg.y & N_BIT)
        cpu_reg.status.negative = 1;
    exec_done = TRUE;
    return TRUE;
}

int func_LSR(void) {
    return FALSE;
}

int func_NOP(void) {
    return FALSE;
}

int func_ORA(void) {
    return FALSE;
}

int func_PHA(void) {
    return FALSE;
}

int func_PHP(void) {
    return FALSE;
}

int func_PLA(void) {
    return FALSE;
}

int func_PLP(void) {
    return FALSE;
}

int func_ROL(void) {
    return FALSE;
}

int func_ROR(void) {
    return FALSE;
}

int func_RTI(void) {
    return FALSE;
}

int func_RTS(void) {
    return FALSE;
}

int func_SBC(void) {
    return FALSE;
}

int func_SEC(void) {
    return FALSE;
}

int func_SED(void) {
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

int func_STA(void) {
    return FALSE;
}

int func_STX(void) {
    return FALSE;
}

int func_STY(void) {
    return FALSE;
}

int func_TAX(void) {
    return FALSE;
}

int func_TAY(void) {
    return FALSE;
}

int func_TSX(void) {
    return FALSE;
}

int func_TXA(void) {
    return FALSE;
}

int func_TXS(void) {
    return FALSE;
}

int func_TYA(void) {
    return FALSE;
}

/*
 * decode6502:
 * return execution cycle count
 * */
int decode6502(unsigned char inst) {

    struct opcode_map * omap = &opcode_list[inst];
    if (omap->func == NULL) {
        return FALSE;
    }

    dprint("decode inst: %02x > %s, %d cycle, %d len\n", 
            inst, omap->mnemonic, omap->cycle, omap->inst_len);

    current_inst = omap;
    current_exec_index = 0;

    return TRUE;
}
int test_and_set_exec(void) {
    int ret;
    ret = exec_done;
    if (exec_done)
        exec_done = FALSE;
    return ret;
}

int execute6502(void) {
    int ret;
    ret = current_inst->func();
    current_exec_index++;
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

void dump_6502(int full) {
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
    //printf("data:     %02x\n", cpu_data_buffer);
}


int init_6502core(void) {
    memset(&cpu_reg, 0, sizeof(struct cpu_6502));
    current_inst = NULL;
    current_exec_index = 0;
    exec_done = FALSE;
    return TRUE;
}

