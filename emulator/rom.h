#ifndef __rom_h__
#define __rom_h__

#include <stdio.h>

void set_rom_addr(unsigned short addr);
unsigned char get_rom_data(void);
void set_rom_data(unsigned char data);
void set_rom_ce_pin(int ce);

int load_rom_file(FILE* cartridge, int num_rom_bank);
int init_rom(void);
void clean_rom(void);

#endif /*__rom_h__*/

