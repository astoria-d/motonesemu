
#include <stdlib.h>

#include "tools.h"
#include "clock.h"

static int rt_signal;
static timer_t cpu_clock_timer;

struct clock_handler {
    struct slist l;
    clock_func_t *handler;
};

static struct clock_handler *handler_list;

static void cpu_clock_loop(int arg) {
    struct clock_handler *ch;

    dprint("loop...\n");
    ch = handler_list;
    while (ch != NULL) {
        if (!ch->handler())
            return;
        ch = (struct clock_handler*)ch->l.next;
    }

    return;
}


int start_clock(void) {
    int ret;
    int sec, nsec;

    sec = CPU_CLOCK_SEC;
    nsec = CPU_CLOCK_NSEC;
    ret = register_timer(sec, nsec, cpu_clock_loop, &cpu_clock_timer);
    return ret == TRUE;
}

int register_clock_hander(clock_func_t *handler) {
    struct clock_handler *ch;

    ch = malloc(sizeof(struct clock_handler));
    ch->l.next = NULL;
    ch->handler = handler;

    if (handler_list == NULL) {
        handler_list = ch;
    }
    else {
        slist_add_tail(&handler_list->l, &ch->l);
    }
    return TRUE;
}

int emu_timer_init(void) {
    rt_signal = SIGRTMIN;
    //rt_signal = SIGALRM;
    return TRUE;
}

int register_timer(unsigned long int_sec, unsigned long int_nanosec, __sighandler_t func, 
        timer_t *timerId) {
    struct sigaction    sigact;
    struct itimerspec   itval;
    struct sigevent sev;


    //register handler
    sigact.sa_handler = func;
    sigact.sa_flags = 0;
    sigemptyset(&sigact.sa_mask);

    if(sigaction(rt_signal,&sigact,NULL) == -1)
    {
        return FALSE;
    }

    //create timer
    itval.it_interval.tv_sec = int_sec;
    itval.it_interval.tv_nsec = int_nanosec;
    itval.it_value.tv_sec = int_sec;
    itval.it_value.tv_nsec = int_nanosec;

    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = rt_signal;
    sev.sigev_value.sival_ptr = timerId;

    rt_signal++;

    if(timer_create(CLOCK_REALTIME, &sev, timerId) == -1)
    {
        return FALSE;
    }
    if(timer_settime(*timerId,0,&itval,NULL) == -1)
    {
        return FALSE;
    }
    return TRUE;
}

int init_clock(void) {
    handler_list = NULL;
    cpu_clock_timer = 0;
    return TRUE;
}

void clean_clock(void) {
    struct clock_handler *ch = handler_list;

    timer_delete(cpu_clock_timer);

    while (ch != NULL) {
        struct clock_handler *pp = ch;
        ch = (struct clock_handler*)ch->l.next;
        free(pp);
    }
    dprint("clean_clock done.\n");
    

}

