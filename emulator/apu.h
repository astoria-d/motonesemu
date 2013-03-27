#ifndef __apu_h__
#define __apu_h__

void set_apu_addr(unsigned short addr);
unsigned char get_apu_data(void);
void set_apu_data(unsigned char data);

void set_apu_start(int ce);

#endif /*__apu_h__*/

