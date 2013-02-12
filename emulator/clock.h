#ifndef __clock_h__
#define __clock_h__

typedef int (clock_func_t) (void);

int init_clock(void);
void clean_clock(void);

int register_clock_hander(clock_func_t *handler);
int start_clock(void);

#endif /*__clock_h__*/

