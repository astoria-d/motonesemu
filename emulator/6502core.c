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

static struct opcode_map *current_inst;
static int current_exec_index;

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

int func_LDX(void) {
    return FALSE;
}

int func_LDY(void) {
    return FALSE;
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

int func_SEI(void) {
    return FALSE;
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
int decode6502(unsigned char inst, int *cycle_cnt, int *inst_len) {

    struct opcode_map * omap = &opcode_list[inst];
    if (omap->func == NULL) {
        return FALSE;
    }

    dprint("decode inst: %02x > %s, %d cycle, %d len\n", 
            inst, omap->mnemonic, omap->cycle, omap->inst_len);
    *cycle_cnt = omap->cycle;
    *inst_len = omap->inst_len;

    current_inst = omap;
    current_exec_index = 0;

    return TRUE;
}

int execute6502(void) {
    current_exec_index++;
    return current_inst->func();
}

int init_6502core(void) {
    current_inst = NULL;
    current_exec_index = 0;
    return TRUE;
}

