#ifndef __clock_h__
#define __clock_h__

#include <signal.h>
#include <time.h>

typedef int (clock_func_t) (void);

int init_clock(void);
void clean_clock(void);

int register_clock_hander(clock_func_t *handler);
int start_clock(void);

int emu_timer_init(void);
int register_timer(unsigned long int_sec, unsigned long int_nanosec, 
        __sighandler_t func, timer_t *timerid);

#define DEB_SLOW

#ifdef DEB_SLOW
#define CPU_CLOCK_SEC   0
#define CPU_CLOCK_NSEC  100000000
#else
#define CPU_CLOCK_SEC   0
#define CPU_CLOCK_NSEC  100
#endif

#endif /*__clock_h__*/

