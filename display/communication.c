#include <time.h>
#include <pthread.h>
#include <unistd.h>

#include "tools.h"
#include "vga.h"

void draw_point(int x, int y, char r, char g, char b);
int window_ready(void);

static pthread_t com_thread_id;

static void *com_loop(void* arg) {
    //struct timespec ts = {0, 1};
    int x, y;
    char r, g, b;

    x = y = 0;
    r = g = b = 0;
    while (!window_ready());

    while (1) {
        //nanosleep(&ts, NULL);
        draw_point(x % VGA_WIDTH, y % VGA_HEIGHT, 
                r, g, b);
        if (++x % VGA_WIDTH == 0) {
            x = 0;
            if (++y % VGA_HEIGHT == 0) {
                y = 0;
            }
        }
        r++;
        g += 2;
        b += 3;
    }
    return NULL;
}


int comm_init(void) {
    int ret;
    pthread_attr_t attr;

    ret = pthread_attr_init(&attr);
    if (ret != RT_OK)
        return FALSE;

    com_thread_id = 0;
    ret = pthread_create(&com_thread_id, &attr, com_loop, NULL);
    if (ret != RT_OK)
        return FALSE;

    return TRUE;
}

