#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "tools.h"
#include "clock.h"

#define SIG_CPU_CLOCK SIGRTMIN

//#define USE_TIMER_SIG

#ifdef USE_TIMER_SIG
#define RETURN_HANDLER  return
#else
static pthread_t cpu_thread_id;
#define RETURN_HANDLER  return NULL
#endif

static timer_t cpu_clock_timer;

struct clock_handler {
    struct slist l;
    clock_func_t *handler;
};

static struct clock_handler *handler_list;

#ifdef USE_TIMER_SIG
static void cpu_clock_loop(int arg) {
#else
static void* cpu_clock_loop(void* arg) {
#endif
    struct clock_handler *ch;

#ifndef USE_TIMER_SIG
    struct timespec ts = {CPU_CLOCK_SEC, CPU_CLOCK_NSEC};
    while(TRUE) {
#endif
    dprint("clock...\n");
    ch = handler_list;
    while (ch != NULL) {
        if (!ch->handler())
            RETURN_HANDLER;
        ch = (struct clock_handler*)ch->l.next;
    }
#ifndef USE_TIMER_SIG
    nanosleep(&ts, NULL);
    }
#endif

    RETURN_HANDLER;
}


int start_clock(void) {
    int ret;
    struct sigaction    sigact;

#ifdef USE_TIMER_SIG
    //register handler
    sigact.sa_handler = cpu_clock_loop;
    sigact.sa_flags = 0;
    sigemptyset(&sigact.sa_mask);

    if(sigaction(SIG_CPU_CLOCK, &sigact, NULL) == -1)
    {
        return FALSE;
    }
    ret = start_cpu_clock();
#else
    {
        pthread_attr_t attr;
        ret = pthread_attr_init(&attr);
        if (ret != RT_OK)
            return FALSE;

        cpu_thread_id = 0;
        ret = pthread_create(&cpu_thread_id, &attr, cpu_clock_loop, NULL);
        if (ret != RT_OK)
            return FALSE;
    }
#endif

    return ret;
}

int pause_cpu_clock(void) {
#ifdef USE_TIMER_SIG
    return 0 == timer_delete(cpu_clock_timer);
#endif
}

int start_cpu_clock(void) {
#ifdef USE_TIMER_SIG
    struct sigevent sev;
    struct itimerspec   itval;
    int int_sec, int_nanosec;

    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIG_CPU_CLOCK;
    sev.sigev_value.sival_ptr = &cpu_clock_timer;

    int_sec = CPU_CLOCK_SEC;
    int_nanosec = CPU_CLOCK_NSEC;
    itval.it_interval.tv_sec = int_sec;
    itval.it_interval.tv_nsec = int_nanosec;
    itval.it_value.tv_sec = int_sec;
    itval.it_value.tv_nsec = int_nanosec;

    if(timer_create(CLOCK_REALTIME, &sev, &cpu_clock_timer) == -1)
    {
        return FALSE;
    }
    if(timer_settime(cpu_clock_timer, 0, &itval, NULL) == -1)
    {
        return FALSE;
    }
#endif
    return TRUE;
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

#if 0
int register_timer(unsigned long int_sec, unsigned long int_nanosec, __sighandler_t func, 
        int signum, timer_t *timerId) {
    struct sigaction    sigact;
    struct itimerspec   itval;
    struct sigevent sev;


    //register handler
    sigact.sa_handler = func;
    sigact.sa_flags = 0;
    sigemptyset(&sigact.sa_mask);

    if(sigaction(signum,&sigact,NULL) == -1)
    {
        return FALSE;
    }

    //create timer
    itval.it_interval.tv_sec = int_sec;
    itval.it_interval.tv_nsec = int_nanosec;
    itval.it_value.tv_sec = int_sec;
    itval.it_value.tv_nsec = int_nanosec;

    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = signum;
    sev.sigev_value.sival_ptr = timerId;

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
#endif

int init_clock(void) {
    handler_list = NULL;
    cpu_clock_timer = 0;
    return TRUE;
}

void clean_clock(void) {
    void* ret;
    struct clock_handler *ch = handler_list;

#ifdef USE_TIMER_SIG
    timer_delete(cpu_clock_timer);
#else
    pthread_join(cpu_thread_id, &ret);
#endif

    while (ch != NULL) {
        struct clock_handler *pp = ch;
        ch = (struct clock_handler*)ch->l.next;
        free(pp);
    }
    dprint("clean_clock done.\n");
    

}

