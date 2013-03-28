#ifndef __clock_h__
#define __clock_h__

typedef int (clock_func_t) (void);

int init_clock(void);
void clean_clock(void);

int register_clock_hander(clock_func_t *handler, int devide);
int start_clock(void);
int pause_cpu_clock(void);
int start_cpu_clock(void);

/*int register_timer(unsigned long int_sec, unsigned long int_nanosec, 
        __sighandler_t func, int signum, timer_t *timerid);*/

#define DEB_SLOW
#undef DEB_SLOW


#define BASE_CLOCK      21477270L
#define CPU_DEVIDER     12
#define PPU_DEVIDER     4

#define CPU_CLOCK_FREQ  (BASE_CLOCK / CPU_DEVIDER)
#define PPU_CLOCK_FREQ  (BASE_CLOCK / PPU_DEVIDER)

#define BASE_CLOCK_NSEC  (1000000000L / BASE_CLOCK)


#endif /*__clock_h__*/

