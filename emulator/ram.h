#ifndef __ram_h__
#define __ram_h__

void set_ram_addr(unsigned short addr);
unsigned char get_ram_data(void);
void set_ram_data(unsigned char data);

void set_ram_ce_pin(int ce);
void set_ram_oe_pin(int oe);
void set_ram_we_pin(int we);

int init_ram(void);
void clear_ram(void);

#endif /*__ram_h__*/

