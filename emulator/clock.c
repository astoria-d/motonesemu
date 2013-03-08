#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <limits.h>
#include <errno.h>

#include "tools.h"
#include "clock.h"

static int exit_loop;
static pthread_t cpu_thread_id;

struct clock_handler {
    struct slist l;
    clock_func_t *handler;
};

static struct clock_handler *handler_list;

#define NANOMAX (1000000000 - 1)

static void* cpu_clock_loop(void* arg) {
    struct clock_handler *ch;
    struct timespec begin;
    struct timespec end;

    dprint("cpu clock started.\n");

    while (!exit_loop) {
        long sec;
        long nsec;

        clock_gettime(CLOCK_REALTIME, &begin);
        dprint("-----------------\nclock ");
        ch = handler_list;
        while (ch != NULL) {
            if (!ch->handler())
                return NULL;
            ch = (struct clock_handler*)ch->l.next;
        }
        
        clock_gettime(CLOCK_REALTIME, &end);
        //calcurate sleep time.
        if (end.tv_sec < begin.tv_sec )
            sec = LONG_MAX - begin.tv_sec + end.tv_sec + 1;
        else
            sec = end.tv_sec - begin.tv_sec;

        if (end.tv_nsec < begin.tv_nsec) 
            nsec = NANOMAX - begin.tv_nsec + end.tv_nsec + 1;
        else
            nsec = end.tv_nsec - begin.tv_nsec;

        //dprint("sec:%d, nsec:%d\n", sec, nsec);
        if (sec < CPU_CLOCK_SEC || nsec < CPU_CLOCK_NSEC) {
            struct timespec ts;
            int ret;
            ts.tv_sec = sec > CPU_CLOCK_SEC ? 0 : CPU_CLOCK_SEC - sec;
            ts.tv_nsec = nsec > CPU_CLOCK_NSEC ? 0 : CPU_CLOCK_NSEC - nsec;

            ret = nanosleep(&ts, NULL);
            //dprint("sleep %d sec:%d, nsec:%d, err:%d\n", ret, ts.tv_sec, ts.tv_nsec, errno);
        }
    }

    return NULL;
}

int start_clock(void) {
    int ret;
    pthread_attr_t attr;

    ret = pthread_attr_init(&attr);
    if (ret != RT_OK)
        return FALSE;

    cpu_thread_id = 0;
    ret = pthread_create(&cpu_thread_id, &attr, cpu_clock_loop, NULL);
    if (ret != RT_OK)
        return FALSE;

    return TRUE;
}

static void end_loop(void) {
    void* ret;
    exit_loop = TRUE;

    //join the running thread.
    pthread_join(cpu_thread_id, &ret);

    dprint("clock thread joined.\n");

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

int init_clock(void) {
    exit_loop = FALSE;
    handler_list = NULL;
    return TRUE;
}

void clean_clock(void) {
    struct clock_handler *ch = handler_list;

    end_loop();

    while (ch != NULL) {
        struct clock_handler *pp = ch;
        ch = (struct clock_handler*)ch->l.next;
        free(pp);
    }
    dprint("clean_clock done.\n");
    

}

