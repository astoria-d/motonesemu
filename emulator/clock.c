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
    int devider;
    int cnt;
};

static struct clock_handler *handler_list;

#define NANOMAX (1000000000L - 1)

static void* cpu_clock_loop(void* arg) {
    struct clock_handler *ch;

    dprint("cpu clock started.\n");

    while (!exit_loop) {

        //dprint("-----------------\nclock ");
        ch = handler_list;
        while (ch != NULL) {
            if (ch->cnt == 0) {
                if (!ch->handler())
                    return NULL;
            }
            if (ch->cnt++ == ch->devider)
                ch->cnt = 0;
            ch = (struct clock_handler*)ch->l.next;
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

    dprint("cpu clock thread joined.\n");

}

int register_clock_hander(clock_func_t *handler, int devide) {
    struct clock_handler *ch;

    ch = malloc(sizeof(struct clock_handler));
    ch->l.next = NULL;
    ch->handler = handler;
    ch->devider = devide;
    ch->cnt = 0;

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
    //dprint("clean_clock done.\n");
}

