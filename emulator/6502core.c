#include "tools.h"

/*
 * instruction consists of following format.
 * aaabbbcc
 * aaa and cc determins opcode
 * bbb determins addr mode.
 *
 * ref:
 * http://www.llx.com/~nparker/a2/opcodes.html
 *
 * */
struct mem_opcode_fmt {
    unsigned int aaa:3;
    unsigned int bbb:3;
    unsigned int cc:2;
};

struct cond_br_opcode_fmt {
    unsigned int xx:2;
    unsigned int y:1;
    unsigned int one:5;
};

struct single_opcode_fmt {
    unsigned int aaaa:4;
    unsigned int eight:4;
};

/*
 * addressing mode for group 1 instruction
 * addressing mode:
 *
 * Zero Page
 * Zero Page, X
 * Zero Page, Y
 * Absolute
 * Absolute, X
 * Absolute, Y
 * Indirect
 * Implied
 * Accumulator
 * Immediate
 * Relative
 * (Indirect, X)
 * (Indirect), Y
 * */

/*(zero page, x)*/
#define AM_GP1_INDR_INDX    0

#define AM_GP1_ZP       1
#define AM_GP1_IMM      2
#define AM_GP1_ABS      3

/*(zero page), y*/
#define AM_GP1_INDX_INDR    4

#define AM_GP1_ZP_X     5
#define AM_GP1_ABS_X    6
#define AM_GP1_ABS_Y    7

#define AM_GP2_IMM      0
#define AM_GP2_ZP       1
#define AM_GP2_ACC      2
#define AM_GP2_ABS      3
#define AM_GP2_ZP_X     5
#define AM_GP2_ABS_X    7

#define AM_GP3_IMM      0
#define AM_GP3_ZP       1
#define AM_GP3_ABS      3
#define AM_GP3_ZP_X     5
#define AM_GP3_ABS_X    7

static int decode_gp1(struct mem_opcode_fmt fmt, int *cycle, int *len) {

    *cycle = 0;
    switch (fmt.aaa) {
        case 0:
            dprint("ORA\n");
            break;

        case 1:
            dprint("AND\n");
            break;

        case 2:
            dprint("EOR\n");
            break;

        case 3:
            dprint("ADC\n");
            break;

        case 4:
            dprint("STA\n");
            break;

        case 5:
            dprint("LDA\n");
            break;

        case 6:
            dprint("CMP\n");
            break;

        case 7:
            dprint("SBC\n");
        default:
            break;
    }

    *len = 2;
    switch (fmt.bbb) {
        case AM_GP1_ZP:
            *cycle = 3;
            break;

        case AM_GP1_IMM:    
            *cycle = 2;
            break;

        case AM_GP1_ABS :  
            *len = 3;
        case AM_GP1_ZP_X:
            *cycle = 4;
            break;

        case AM_GP1_ABS_X: 
        case AM_GP1_ABS_Y:
            *cycle = 7;
            *len = 3;
            break;

        case AM_GP1_INDR_INDX:
            *cycle = 6;
            break;

        case AM_GP1_INDX_INDR   :
            *cycle = 5;
            break;

        default:
            return FALSE;
    }

    return TRUE;
}

static int decode_gp2(struct mem_opcode_fmt fmt, int *cycle, int *len) {
    *cycle = 0;
    switch (fmt.aaa) {
        case 0:
            dprint("ASL\n");
            break;

        case 1:
            dprint("ROL\n");
            break;

        case 2:
            dprint("LSR\n");
            break;

        case 3:
            dprint("ROR\n");
            break;

        case 4:
            dprint("STX\n");
            break;

        case 5:
            dprint("LDX\n");
            break;

        case 6:
            dprint("DEC\n");
            break;

        case 7:
            dprint("INC\n");
        default:
            break;
    }

    *len = 2;
    switch (fmt.bbb) {
        case AM_GP2_ZP:
            *cycle = 3;
            break;

        case AM_GP2_ACC:    
            *len = 1;
        case AM_GP2_IMM:    
            *cycle = 2;
            break;

        case AM_GP2_ABS :  
            *len = 3;
        case AM_GP2_ZP_X:
            *cycle = 4;
            break;

        case AM_GP2_ABS_X: 
            *len = 3;
            *cycle = 7;
            break;

        default:
            return FALSE;
    }

    return TRUE;
}

static int decode_gp3(struct mem_opcode_fmt fmt, int *cycle, int *len) {
    *cycle = 0;
    switch (fmt.aaa) {
        case 1:
            dprint("BIT\n");
            break;

        case 2:
            dprint("JMP\n");
            break;

        case 3:
            dprint("JMP(abs)\n");
            break;

        case 4:
            dprint("STY\n");
            break;

        case 5:
            dprint("LDY\n");
            break;

        case 6:
            dprint("CPY\n");
            break;

        case 7:
            dprint("CPX\n");
            break;

        case 0:
        default:
            return FALSE;
    }

    *len = 2;
    switch (fmt.bbb) {
        case AM_GP3_ZP:
            *cycle = 3;
            break;

        case AM_GP3_IMM:    
            *cycle = 2;
            break;

        case AM_GP3_ABS :  
            *len = 3;
        case AM_GP3_ZP_X:
            *cycle = 4;
            break;

        case AM_GP3_ABS_X: 
            *cycle = 7;
            *len = 3;
            break;

        default:
            return FALSE;
    }

    return TRUE;
}

static int decode_cond_br(unsigned char inst, int *cycle, int *len) {
    *cycle = 2;
    *len = 2;
    switch (inst) {
        case 0x10:
            dprint("BPL\n");
            break;
        case 0x30:
            dprint("BMI\n");
            break;
        case 0x50:
            dprint("BVC\n");
            break;
        case 0x70:
            dprint("BVS\n");
            break;
        case 0x90:
            dprint("BCC\n");
            break;
        case 0xB0:
            dprint("BCS\n");
            break;
        case 0xD0:
            dprint("BNE\n");
            break;
        case 0xF0:
            dprint("BEQ\n");
            break;
        case 0x00:
            dprint("BRK\n");
            *cycle = 7;
            *len = 1;
            break;
        case 0x20:
            dprint("JSR abs\n");
            *cycle = 6;
            *len = 3;
            break;
        case 0x40:
            dprint("RTI\n");
            *cycle = 6;
            *len = 1;
            break;
        case 0x60:
            dprint("RTS\n");
            *cycle = 6;
            *len = 1;
            break;
        default:
            return FALSE;
    }

    return TRUE;
}

static int decode_single(unsigned char inst, int *cycle, int *len) {
    *cycle = 2;
    *len = 1;
    switch (inst) {
        case 0x08:
            dprint("PHP\n");
            *cycle = 3;
            break;
        case 0x28:
            dprint("PLP\n");
            *cycle = 4;
            break;
        case 0x48:
            dprint("PHA\n");
            *cycle = 3;
            break;
        case 0x68:
            dprint("PLA\n");
            *cycle = 4;
            break;
        case 0x88:
            dprint("DEY\n");
            break;
        case 0xA8:
            dprint("TAY\n");
            break;
        case 0xC8:
            dprint("INY\n");
            break;
        case 0xE8:
            dprint("INX\n");
            break;
        case 0x18:
            dprint("CLC\n");
            break;
        case 0x38:
            dprint("SEC\n");
            break;
        case 0x58:
            dprint("CLI\n");
            break;
        case 0x78:
            dprint("SEI\n");
            break;
        case 0x98:
            dprint("TYA\n");
            break;
        case 0xb8:
            dprint("CLV\n");
            break;
        case 0xD8:
            dprint("CLD\n");
            break;
        case 0xF8:
            dprint("SED\n");
            break;
        case 0x8A:
            dprint("TXA\n");
            break;
        case 0x9A:
            dprint("TXS\n");
            break;
        case 0xAA:
            dprint("TAX\n");
            break;
        case 0xBA:
            dprint("TSX\n");
            break;
        case 0xCA:
            dprint("DEX\n");
            break;
        case 0xEA:
            dprint("NOP\n");
            break;
        default:
            return FALSE;
    }

    return TRUE;
}

/*
 * decode6502:
 * return execution cycle count
 * */
int decode6502(unsigned char inst, int *cycle_cnt, int *inst_len) {
    int ret = FALSE;
    struct mem_opcode_fmt* m_fmt = (struct mem_opcode_fmt*)&inst;
    struct cond_br_opcode_fmt* c_fmt = (struct cond_br_opcode_fmt*)&inst;
    struct single_opcode_fmt* s_fmt = (struct single_opcode_fmt*)&inst;


    if (m_fmt->cc == 1)
        ret = decode_gp1(*m_fmt, cycle_cnt, inst_len);
    else if (m_fmt->cc == 2)
        ret = decode_gp2(*m_fmt, cycle_cnt, inst_len);
    else if (m_fmt->cc == 0)
        ret = decode_gp3(*m_fmt, cycle_cnt, inst_len);

    /*conditional branc group*/
    else if (c_fmt->one == 0x10)
        ret = decode_cond_br(inst, cycle_cnt, inst_len);

    /*single byte inst group*/
    else if (s_fmt->eight == 0x08 || s_fmt->eight == 0x0a)
        ret = decode_single(inst, cycle_cnt, inst_len);

    return ret;
}

