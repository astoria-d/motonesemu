#ifndef __clock_h__
#define __clock_h__

typedef int (clock_handler_t) (void);

void clock_loop(void);
int init_clock(void);

int register_clock_hander(clock_handler_t *handler);

#endif /*__clock_h__*/

