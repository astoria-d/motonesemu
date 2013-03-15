#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

#include "clock.h"
#include "tools.h"
#include "vga.h"
#include "ppucore.h"

void set_vga_base(unsigned char* base);
void *vga_shm_get(void);
void vga_shm_free(void* addr);
void release_bus(void);

struct ppu_cpu_pin {
    unsigned int ce     :1;     /*chip enable*/
    unsigned int rw     :1;     /*assert on write.*/
    unsigned int vblank :1;     /*connected to nmi*/
};

struct ppu_cart_pin {
    unsigned int rd     :1;     /*read*/
    unsigned int wr     :1;     /*write.*/
};

static struct ppu_cpu_pin  ppu_pin;
static struct ppu_cart_pin cart_pin;

static unsigned short ppu_addr;
static unsigned char ppu_data;

static pthread_t ppu_thread_id;
static int ppu_end_loop;
static sem_t ppu_sem_id;
static unsigned char* vga_shared_buf;


/*
 * ppucore r/w func ptr.
 * */
typedef void (ppu_write_t)(unsigned char);
typedef unsigned char (ppu_read_t)(void);

static void null_write(unsigned char);
static unsigned char null_read(void);

static ppu_write_t *ppucore_write_func[8] = {
    ppu_ctrl1_set,
    ppu_ctrl2_set,
    null_write,
    sprite_data_set,
    sprite_addr_set,
    ppu_scroll_set,
    ppu_vram_addr_set,
    ppu_vram_data_set
};

static ppu_read_t *ppucore_read_func[8] = {
    null_read,
    null_read,
    ppu_status_get,
    null_read,
    null_read,
    null_read,
    null_read,
    ppu_vram_data_get
};

/*
* JAPAN/US uses NTSC standard.
* 
* NTSC: 
* ---------------------------------------------------------
* Frames per second                            60 
* Time per frame (milliseconds)                16.67 
* Scanlines per frame (of which is V-Blank)    262 (20) 
* CPU cycles per scanline                      113.33 
* Resolution                                   256 x 224 
* CPU speed                                    1.79 MHz 
*
* PPU clock                                    21.48 Mhz
* */

void set_ppu_addr(unsigned short addr) {
    ppu_addr = addr;
}

unsigned char get_ppu_data(void) {
    return ppu_data;
}

void set_ppu_data(unsigned char data) {
    ppu_data = data;
}

/*
 * write: rw = 1
 * read: rw = 0
 * */
void set_ppu_rw_pin(int rw) {
    ppu_pin.rw = rw;
}

void set_ppu_ce_pin(int ce) {
    ppu_pin.ce = ce;
    //let ram i/o on the bus.
    if (ce)
        sem_post(&ppu_sem_id);
}

/*dummy I/O func*/
static void null_write(unsigned char d){}
static unsigned char null_read(void){return 0;}

static void *ppu_loop(void* arg) {
    while (!ppu_end_loop) {
        sem_wait(&ppu_sem_id);
        if (ppu_pin.ce) {
            if (ppu_pin.rw) {
                //write cycle
                ppucore_write_func[ppu_addr](ppu_data);
            }
            else {
                //read cycle
                ppu_data = ppucore_read_func[ppu_addr]();
            }
            release_bus();
        }
    }
    return NULL;
}

int init_ppu(void) {
    int ret;
    pthread_attr_t attr;

    ppu_end_loop = FALSE;

    ppu_pin.ce = 0;
    ppu_pin.rw = 0;
    cart_pin.rd = 0;
    cart_pin.wr = 0;

    ppu_addr = 0;
    ppu_data = 0;

    /* get vga shared memory */
    if((vga_shared_buf = (unsigned char*)vga_shm_get()) == NULL)
    {
        fprintf(stderr, "error attaching shared memory.\n");
        return FALSE;
    }

    memset(vga_shared_buf, 0, VGA_SHM_SIZE);
    set_vga_base((unsigned char*)vga_shared_buf);

    ret = ppucore_init();
    if (ret == FALSE) {
        return FALSE;
    }

    ret = sem_init(&ppu_sem_id, 0, 0);
    if (ret != RT_OK) {
        return FALSE;
    }

    ret = pthread_attr_init(&attr);
    if (ret != RT_OK) {
        return FALSE;
    }

    ppu_thread_id = 0;
    ret = pthread_create(&ppu_thread_id, &attr, ppu_loop, NULL);
    if (ret != RT_OK) {
        return FALSE;
    }

    return TRUE;
}

void clean_ppu(void) {
    void* ret;

    ppu_end_loop = TRUE;
    sem_post(&ppu_sem_id);
    pthread_join(ppu_thread_id, &ret);
    sem_destroy(&ppu_sem_id);
    dprint("ppu thread joined.\n");

    clean_ppucore();
    vga_shm_free(vga_shared_buf);

}


