
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#include "tools.h"
#include "clock.h"

static int exit_loop;
static pthread_t clock_thread_id;

struct clock_handler {
    struct slist l;
    clock_func_t *handler;
};

static struct clock_handler *handler_list;

#define DEB_SLOW

static void *clock_loop(void* arg) {
    struct timespec ts;
    struct clock_handler *ch;

#ifdef DEB_SLOW
    ts.tv_sec = 0;
    ts.tv_nsec = 100000000;
#else
    ts.tv_sec = 0;
    ts.tv_nsec = 1;
#endif

    while (!exit_loop) {
        //dprint("loop...\n");
        ch = handler_list;
        while (ch != NULL) {
            if (!ch->handler())
                return NULL;
            ch = (struct clock_handler*)ch->l.next;
        }
        nanosleep(&ts, NULL);
    }

    return NULL;
}

int start_clock(void) {
    int ret;
    pthread_attr_t attr;

    ret = pthread_attr_init(&attr);
    if (ret != RT_OK)
        return FALSE;

    ret = pthread_create(&clock_thread_id, &attr, clock_loop, NULL);
    return ret == TRUE;
}

static void end_loop(void) {
    void* ret;
    exit_loop = TRUE;

    //join the running thread.
    pthread_join(clock_thread_id, &ret);

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
