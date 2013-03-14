#ifndef __ppu_h__
#define __ppu_h__

#include <stdio.h>

void set_ppu_addr(unsigned short addr);
unsigned char get_ppu_data(void);
void set_ppu_data(unsigned char data);

void set_ppu_ce_pin(int ce);
void set_ppu_rw_pin(int rw);

#endif /*__ppu_h__*/

