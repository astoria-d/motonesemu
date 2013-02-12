
#include <time.h>
#include "tools.h"
#include "clock.h"

static int exit_loop;

struct clock_handler {
    struct slist l;
    clock_handler_t *handler;
};

static struct clock_handler *handler_list;

void clock_loop(void) {
    struct timespec ts;
    struct clock_handler *ch;

    ts.tv_sec = 0;
    ts.tv_nsec = 1;

    while (!exit_loop) {
        ch = handler_list;
        while (ch != NULL) {
            if (!ch->handler())
                break;
            ch = (struct slist*)ch->l.next;
        }
        nanosleep(&ts, NULL);
    }
}

int register_clock_hander(clock_handler_t *handler) {
    return TRUE;
}

int init_clock(void) {
    exit_loop = FALSE;
    handler_list = NULL;
    return TRUE;
}

