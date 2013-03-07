#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

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
    int sock;


    posx = posy = old_x = old_y = 0;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1) {
        fprintf(stderr, "error socket!\n");
        return NULL;
    }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(DISPLAY_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    bind(sock, (struct sockaddr *)&addr, sizeof(addr));

    while (1) {
        //FILE* fifo;
        int ret;
        //struct vga_pulse data;
        struct rgb15 disp_data[VGA_WIDTH][VGA_HEIGHT];
        
        //ret = fread (&data, 1, sizeof(data), fifo);
        ret = recv(sock, &disp_data, sizeof(disp_data), 0);
        dprint("received...\n");
        fflush(stdout);
        if (ret == sizeof(disp_data)) {
            //fclose(fifo);
            continue;
        }
        dprint("ok...\n");


            //dprint("[%d, %d] rgb=%d:%d:%d\n", posx, posy, data.r, data.g, data.b);
            //fflush(stdout);

        for (posy = 0; posy < VGA_HEIGHT; posy++) {
            for (posx = 0; posx < VGA_WIDTH; posx++) {
                set_pixel_color(posx, posy, 
                        to16bit(disp_data[posx][posy].r), 
                        to16bit(disp_data[posx][posy].g), 
                        to16bit(disp_data[posx][posy].b));
            }
        }
    }
    close(sock);
    /*
    */

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

