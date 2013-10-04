#ifndef __bus_h__
#define __bus_h__

void set_bus_addr(unsigned short addr);
void set_bus_data(unsigned char data);
unsigned char get_bus_data(void);

void start_bus(void);
void end_bus(void);
void set_rw_pin(int rw);

void release_bus(void);
void take_bus(void);

void set_nmi_pin(int val);
int get_nmi_pin(void);

void set_reset_pin(int val);
int get_reset_pin(void);

#endif /*__bus_h__*/

