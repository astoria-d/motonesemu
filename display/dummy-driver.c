#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "tools.h"
#include "vga.h"

static struct rgb15 *disp_data;
void *vga_shm_get(void);

//struct timespec sleep_inteval = {0, 1};

static void pipe_sig_handler(int p) {
    printf("sigpipe!\n");
}


static void init_color1(void) {
    int x,y;
    for (y = 0; y < VGA_HEIGHT; y++) {
        for (x = 0; x < VGA_WIDTH; x++) {
            int pos = x + VGA_WIDTH * y;
            if (x < VGA_WIDTH / 7) {
                //75% white
                disp_data[pos].r = to5bit(0xffff) * 3 / 4;
                disp_data[pos].g = to5bit(0xffff) * 3 / 4;
                disp_data[pos].b = to5bit(0xffff) * 3 / 4;
            }
            else if (x < VGA_WIDTH * 2 / 7) {
                //yellow
                disp_data[pos].r = to5bit(0xffff);
                disp_data[pos].g = to5bit(0xffff);
                disp_data[pos].b = to5bit(0);
            }
            else if (x < VGA_WIDTH * 3 / 7) {
                //cian
                disp_data[pos].r = to5bit(0);
                disp_data[pos].g = to5bit(0xffff);
                disp_data[pos].b = to5bit(0xffff);
            }
            else if (x < VGA_WIDTH * 4 / 7) {
                //green
                disp_data[pos].r = 0;
                disp_data[pos].g = to5bit(0xffff);
                disp_data[pos].b = 0;
            }
            else if (x < VGA_WIDTH * 5 / 7) {
                //magenda
                disp_data[pos].r = to5bit(0xffff);
                disp_data[pos].g = to5bit(0);
                disp_data[pos].b = to5bit(0xffff);
            }
            else if (x < VGA_WIDTH * 6 / 7) {
                //red
                disp_data[pos].r = to5bit(0xffff);
                disp_data[pos].g = 0;
                disp_data[pos].b = 0;
            }
            else if (x < VGA_WIDTH * 7 / 7) {
                //blue
                disp_data[pos].r = 0;
                disp_data[pos].g = 0;
                disp_data[pos].b = to5bit(0xffff);
            }

        }
    }
}

int main(int argc, char** argv) {
    //register signal handler
    //signal(SIGPIPE, SIG_IGN);
    signal(SIGPIPE, pipe_sig_handler);
    
    /* get vga shared memory */
    if((disp_data = (struct rgb15 *)vga_shm_get()) == NULL)
    {
        fprintf(stderr, "error attaching shared memory.\n");
        return -1;
    }

    memset(disp_data, 0, sizeof(VGA_SHM_SIZE));
    init_color1();
    
    return 0;
}


