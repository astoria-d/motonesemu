#ifndef __clock_h__
#define __clock_h__

#include <signal.h>
#include <time.h>

typedef int (clock_func_t) (void);

int init_clock(void);
void clean_clock(void);

int register_clock_hander(clock_func_t *handler);
int start_clock(void);
int pause_cpu_clock(void);
int start_cpu_clock(void);

/*int register_timer(unsigned long int_sec, unsigned long int_nanosec, 
        __sighandler_t func, int signum, timer_t *timerid);*/

#define DEB_SLOW

#ifdef DEB_SLOW
#define CPU_CLOCK_FREQ  10L
#else
#define CPU_CLOCK_FREQ  1789773L
#endif

#define CPU_CLOCK_SEC   (1L / CPU_CLOCK_FREQ)
#define CPU_CLOCK_NSEC  (1000000000L / CPU_CLOCK_FREQ)

#endif /*__clock_h__*/

