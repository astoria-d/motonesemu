#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#include "tools.h"
#include "vga.h"

void draw_point(int x, int y, char r, char g, char b);
void set_pixel_color(int x, int y, int r, int g, int b);
int window_ready(void);

static pthread_t com_thread_id;

static void *com_loop(void* arg) {
    struct timespec ts = {0, 10};
    int x, y;
    int r, g, b;

    r = g = b = 0;

    while (1) {
        for (y = 0; y < VGA_HEIGHT; y++) {
            for (x = 0; x < VGA_WIDTH; x++) {
                set_pixel_color(x, y, r, g, b);
            }
        }
        r = rand();
        g = rand();
        b = rand();
        //dprint("sleep while...\n");
        nanosleep(&ts, NULL);
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

