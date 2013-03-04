#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>

#include "tools.h"
#include "clock.h"

struct rom_pin {
    unsigned int rw     :1;     /*assert on write.*/
    unsigned int ce     :1;     /*chip enable*/
};

static struct rom_pin rom_pin_status;
static unsigned short rom_addr;
static unsigned char rom_data;
static pthread_t rom_thread_id;
static int rom_end_loop;
static sem_t rom_sem_id;

#define ROM_32K 0x8000

unsigned char * rom_buffer;

int load_rom_file(FILE* cartridge, int num_rom_bank) {
    int len;

    rom_buffer = malloc(ROM_32K);
    if (rom_buffer == NULL)
        return FALSE;
    len = fread(rom_buffer, 1, ROM_32K, cartridge);
    if (len != ROM_32K)
        return FALSE;
    return TRUE;
}

void set_rom_addr(unsigned short addr) {
    rom_addr = addr;
}

unsigned char get_rom_data(void) {
    return rom_data;
}

void set_rom_ce_pin(int ce) {
    rom_pin_status.ce = ce;
    //let rom write the value on the bus.
    if (ce)
        sem_post(&rom_sem_id);
}

static void *rom_loop(void* arg) {
    //rom data load delay is 1/10 (dummy interval)
    struct timespec ts = {CPU_CLOCK_SEC, CPU_CLOCK_NSEC / 10};

    while (!rom_end_loop) {
        sem_wait(&rom_sem_id);
        if (rom_pin_status.ce) {
            nanosleep(&ts, NULL);
            rom_data = rom_buffer[rom_addr];
        }
    }
    return NULL;
}

int init_rom(void) {
    int ret;
    pthread_attr_t attr;

    rom_buffer = NULL;
    rom_addr = 0;
    rom_data = 0;
    rom_pin_status.rw = 0;
    rom_pin_status.ce = 0;

    rom_end_loop = FALSE;

    ret = pthread_attr_init(&attr);
    if (ret != RT_OK)
        return FALSE;

    rom_thread_id = 0;
    ret = pthread_create(&rom_thread_id, &attr, rom_loop, NULL);
    if (ret != RT_OK)
        return FALSE;

    ret = sem_init(&rom_sem_id, 0, 0);
    if (ret != RT_OK)
        return FALSE;

    return TRUE;
}

void clear_rom(void) {
    void* ret;
    rom_end_loop = TRUE;
    //join the running thread.
    sem_post(&rom_sem_id);
    pthread_join(rom_thread_id, &ret);

    sem_destroy(&rom_sem_id);
    dprint("rom thread joined.\n");

    if (rom_buffer)
        free(rom_buffer);
}

