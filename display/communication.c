#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "tools.h"
#include "vga.h"

void draw_point(int x, int y, char r, char g, char b);
void set_pixel_color(int x, int y, int r, int g, int b);
int window_ready(void);

static pthread_t com_thread_id;

static int fifo_init(void) {
    int ret;

    //create named fifo.
    ret = mknod(VGA_FIFO, S_IFIFO | 0666, 0);
    if (ret != RT_OK && errno != EEXIST) {
        fprintf(stderr, "error creating pipe!\n");
        return FALSE;
    }

    return TRUE;
}

static void *com_loop(void* arg) {
    int posx, posy, old_x, old_y;


    posx = posy = old_x = old_y = 0;
    /*
    while (1) {
        FILE* fifo;
        int ret;
        struct vga_pulse data;
        
        //dprint("fifo open...");
        fifo= fopen(VGA_FIFO, "r");
        if (fifo == NULL) {
            fprintf(stderr, "error opening fifo!\n");
            return NULL;
        }

        ret = fread (&data, 1, sizeof(data), fifo);
        if (ret == 0) {
            fclose(fifo);
            continue;
        }

        if (data.h_sync ==0) {
            posx=0;
            if (posx != old_x)
                posy++;
            dprint("h_sync=0\n");
        }
        if (data.v_sync ==0) {
            posx=0;
            posy=0;
            dprint("v_sync=0\n");
        }
        if (data.h_sync && data.v_sync) {

            dprint("[%d, %d] rgb=%d:%d:%d\n", posx, posy, data.r, data.g, data.b);
            fflush(stdout);

            set_pixel_color(posx, posy, 
                    to16bit(data.r), to16bit(data.g), to16bit(data.b));
            posx++;
        }
        old_x = posx;
        old_y = posy;
        if (posx == VGA_WIDTH)
            posx = 0;
        if (posy == VGA_HEIGHT)
            posy = 0;

        fclose(fifo);
    }*/

    /*
    */
    while (1) {
        int x, y;
        int r,g,b;
        struct timespec ts = {0, 10};

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

    ret = fifo_init();
    if (!ret)
        return FALSE;

    ret = pthread_attr_init(&attr);
    if (ret != RT_OK)
        return FALSE;

    com_thread_id = 0;
    ret = pthread_create(&com_thread_id, &attr, com_loop, NULL);
    if (ret != RT_OK)
        return FALSE;

    return TRUE;
}

