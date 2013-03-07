#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "tools.h"
#include "vga.h"

void draw_point(int x, int y, char r, char g, char b);
void set_pixel_color(int x, int y, int r, int g, int b);
static struct rgb15 *disp_data;

static pthread_t com_thread_id;

static void *com_loop(void* arg) {
    int posx, posy;

    while (1) {
        struct timespec ts = {0, 100};
        for (posy = 0; posy < VGA_HEIGHT; posy++) {
            for (posx = 0; posx < VGA_WIDTH; posx++) {
                int pos = posx + VGA_WIDTH * posy;
                set_pixel_color(posx, posy, 
                        to16bit(disp_data[pos].r), 
                        to16bit(disp_data[pos].g), 
                        to16bit(disp_data[pos].b));
            }
        }
        nanosleep(&ts, NULL);
    }
    /*
    */

    /*
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
    */

    return NULL;
}

static int shm_init(void) {
    key_t key;
    int   shmid;

    //create shared memory
    key = ftok(VGA_SHM, VGA_SHM_PRJ_ID);
    if (key == -1) {
        fprintf(stderr, "error preparing shared memory.\n");
        return FALSE;
    }

    if((shmid = shmget(key, VGA_SHM_SIZE, IPC_CREAT|IPC_EXCL|0666)) == -1) 
    {
        printf("Shared memory segment exists - opening as client\n");

        /* Segment probably already exists - try as a client */
        if((shmid = shmget(key, VGA_SHM_SIZE, 0)) == -1) 
        {
            fprintf(stderr, "error opening shared memory.\n");
            return FALSE;
        }
    }

    /* Attach (map) the shared memory segment into the current process */
    if((disp_data = (struct rgb15 *)shmat(shmid, 0, 0)) == (struct rgb15*)-1)
    {
        fprintf(stderr, "error attaching shared memory.\n");
        return FALSE;
    }
    return TRUE;
}

int comm_init(void) {
    int ret;
    pthread_attr_t attr;

    ret = shm_init();
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

