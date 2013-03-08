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
int ppucore_init(void);

struct timespec sleep_inteval = {0, 1000000 / VGA_REFRESH_RATE};

int main(int argc, char** argv) {
    int ret;

    ret = ppucore_init();
    if (ret == FALSE) {
        fprintf(stderr, "ppucore init error.\n");
        return -1;
    }

    /* get vga shared memory */
    if((disp_data = (struct rgb15 *)vga_shm_get()) == NULL)
    {
        fprintf(stderr, "error attaching shared memory.\n");
        return -1;
    }

    memset(disp_data, 0, VGA_SHM_SIZE);

    vga_shm_free(disp_data);
    
    return 0;
}


