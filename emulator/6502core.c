#include <libio.h>
#include "tools.h"

typedef int (handler_6502_t) (void);

struct opcode_map {
    unsigned char   opcode;
    char            mnemonic[4];
    handler_6502_t  *func;
    int             addr_mode;
    int             cycle;
    int             inst_len;
};

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

int func_ADC(void) {
    return TRUE;
}

int func_AND(void) {
    return TRUE;
}

int func_ASL(void) {
    return TRUE;
}

int func_BCC(void) {
    return TRUE;
}

int func_BCS(void) {
    return TRUE;
}

int func_BEQ(void) {
    return TRUE;
}

int func_BIT(void) {
    return TRUE;
}

int func_BMI(void) {
    return TRUE;
}

int func_BNE(void) {
    return TRUE;
}

int func_BPL(void) {
    return TRUE;
}

int func_BRK(void) {
    return TRUE;
}

int func_BVC(void) {
    return TRUE;
}

int func_BVS(void) {
    return TRUE;
}

int func_CLC(void) {
    return TRUE;
}

int func_CLD(void) {
    return TRUE;
}

int func_CLI(void) {
    return TRUE;
}

int func_CLV(void) {
    return TRUE;
}

int func_CMP(void) {
    return TRUE;
}

int func_CPX(void) {
    return TRUE;
}

int func_CPY(void) {
    return TRUE;
}

int func_DEC(void) {
    return TRUE;
}

int func_DEX(void) {
    return TRUE;
}

int func_DEY(void) {
    return TRUE;
}

int func_EOR(void) {
    return TRUE;
}

int func_INC(void) {
    return TRUE;
}

int func_INX(void) {
    return TRUE;
}

int func_INY(void) {
    return TRUE;
}

int func_JMP(void) {
    return TRUE;
}

int func_JSR(void) {
    return TRUE;
}

int func_LDA(void) {
    return TRUE;
}

int func_LDX(void) {
    return TRUE;
}

int func_LDY(void) {
    return TRUE;
}

int func_LSR(void) {
    return TRUE;
}

int func_NOP(void) {
    return TRUE;
}

int func_ORA(void) {
    return TRUE;
}

int func_PHA(void) {
    return TRUE;
}

int func_PHP(void) {
    return TRUE;
}

int func_PLA(void) {
    return TRUE;
}

int func_PLP(void) {
    return TRUE;
}

int func_ROL(void) {
    return TRUE;
}

int func_ROR(void) {
    return TRUE;
}

int func_RTI(void) {
    return TRUE;
}

int func_RTS(void) {
    return TRUE;
}

int func_SBC(void) {
    return TRUE;
}

int func_SEC(void) {
    return TRUE;
}

int func_SED(void) {
    return TRUE;
}

int func_SEI(void) {
    return TRUE;
}

int func_STA(void) {
    return TRUE;
}

int func_STX(void) {
    return TRUE;
}

int func_STY(void) {
    return TRUE;
}

int func_TAX(void) {
    return TRUE;
}

int func_TAY(void) {
    return TRUE;
}

int func_TSX(void) {
    return TRUE;
}

int func_TXA(void) {
    return TRUE;
}

int func_TXS(void) {
    return TRUE;
}

int func_TYA(void) {
    return TRUE;
}

/*
 * decode6502:
 * return execution cycle count
 * */
int decode6502(unsigned char inst, int *cycle_cnt, int *inst_len) {

    struct opcode_map * omap = &opcode_list[inst];
    if (omap->func == NULL) {
        return FALSE;
    }

    dprint("decode inst: %02x > %s, %d cycle, %d len\n", 
            inst, omap->mnemonic, omap->cycle, omap->inst_len);
    *cycle_cnt = omap->cycle;
    *inst_len = omap->inst_len;
    return TRUE;
}

