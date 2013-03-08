#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "tools.h"
#include "vga.h"

static struct rgb15 *disp_data;
void *vga_shm_get(void);
void vga_shm_free(void* addr);

struct timespec sleep_inteval = {0, 1000000 / VGA_REFRESH_RATE};

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

static void init_color2(void) {
    int x,y;
    for (y = 0; y < VGA_HEIGHT; y++) {
        for (x = 0; x < VGA_WIDTH; x++) {
            int pos = x + VGA_WIDTH * y;
            if (y < VGA_HEIGHT / 7) {
                //75% white
                disp_data[pos].r = to5bit(0xffff) * 3 / 4;
                disp_data[pos].g = to5bit(0xffff) * 3 / 4;
                disp_data[pos].b = to5bit(0xffff) * 3 / 4;
            }
            else if (y < VGA_HEIGHT * 2 / 7) {
                //yellow
                disp_data[pos].r = to5bit(0xffff);
                disp_data[pos].g = to5bit(0xffff);
                disp_data[pos].b = to5bit(0);
            }
            else if (y < VGA_HEIGHT * 3 / 7) {
                //cian
                disp_data[pos].r = to5bit(0);
                disp_data[pos].g = to5bit(0xffff);
                disp_data[pos].b = to5bit(0xffff);
            }
            else if (y < VGA_HEIGHT * 4 / 7) {
                //green
                disp_data[pos].r = 0;
                disp_data[pos].g = to5bit(0xffff);
                disp_data[pos].b = 0;
            }
            else if (y < VGA_HEIGHT * 5 / 7) {
                //magenda
                disp_data[pos].r = to5bit(0xffff);
                disp_data[pos].g = to5bit(0);
                disp_data[pos].b = to5bit(0xffff);
            }
            else if (y < VGA_HEIGHT * 6 / 7) {
                //red
                disp_data[pos].r = to5bit(0xffff);
                disp_data[pos].g = 0;
                disp_data[pos].b = 0;
            }
            else if (y < VGA_HEIGHT * 7 / 7) {
                //blue
                disp_data[pos].r = 0;
                disp_data[pos].g = 0;
                disp_data[pos].b = to5bit(0xffff);
            }

        }
    }
}

static void move_color1(void) {
    init_color1();
    int x, y;
    static struct rgb15 v_line[VGA_HEIGHT];

    while (1) {
        for (y = 0; y < VGA_HEIGHT; y++) {
            v_line[y] = disp_data[y * VGA_WIDTH];
        }
        for (y = 0; y < VGA_HEIGHT; y++) {
            for (x = 0; x < VGA_WIDTH - 1; x++) {
                int pos = x + VGA_WIDTH * y;
                disp_data[pos] = disp_data[pos + 1];
            }
            disp_data[VGA_WIDTH * y - 1] = v_line[y];
        }
        nanosleep(&sleep_inteval, NULL);
    }
}

static void move_color2(void) {
    init_color2();
    int y;
    static struct rgb15 h_line[VGA_WIDTH];

    while (1) {
        memcpy(h_line, disp_data, sizeof(h_line));
        for (y = 0; y < VGA_HEIGHT - 1; y++) {
            memcpy( disp_data + VGA_WIDTH * y, disp_data + VGA_WIDTH * (y + 1), sizeof(h_line));
        }
        memcpy(disp_data + VGA_WIDTH * (VGA_HEIGHT - 1), h_line, sizeof(h_line));
        nanosleep(&sleep_inteval, NULL);
    }
}

static void print_usage(void) {
    printf("dummy-driver app usage:\n");
    printf("    dummy-driver [1-4]\n");
    printf("    1: vertical color pattern test\n");
    printf("    2: horiontal color pattern test\n");
    printf("    3: vertical color pattern w/ scroll test\n");
    printf("    4: horizontal color pattern w/ scroll test\n");
    printf("    [no option]: reset shared memory\n");
}

int main(int argc, char** argv) {
    print_usage();
    
    /* get vga shared memory */
    if((disp_data = (struct rgb15 *)vga_shm_get()) == NULL)
    {
        fprintf(stderr, "error attaching shared memory.\n");
        return -1;
    }

    memset(disp_data, 0, VGA_SHM_SIZE);

    if (argc > 1) {
        if ( !strcmp(argv[1],"2") )
            init_color2();
        else if ( !strcmp(argv[1],"3") )
            move_color1();
        else if ( !strcmp(argv[1],"4") )
            move_color2();
        else
            init_color1();
    }

    vga_shm_free(disp_data);
    
    return 0;
}


