#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>

#include "tools.h"
#include "clock.h"

void release_bus(void);

struct ram_pin {
    unsigned int ce     :1;     /*chip enable*/
    unsigned int oe     :1;     /*assert on read ready.*/
    unsigned int we     :1;     /*assert on write.*/
};

static struct ram_pin ram_pin_status;
static unsigned short ram_addr;
static unsigned char ram_data;

static pthread_t ram_thread_id;
static int ram_end_loop;
static sem_t ram_sem_id;

#define RAM_2K 0x0800

unsigned char * ram_buffer;

unsigned char dbg_ram_get_byte(unsigned short offset) {
    return ram_buffer[offset];
}
unsigned short dbg_ram_get_short(unsigned short offset) {
    unsigned short ret;
    ret = ram_buffer[offset];
    ret |= (ram_buffer[offset + 1] << 8);
    return ret;
}

void set_ram_addr(unsigned short addr) {
    ram_addr = addr;
}

unsigned char get_ram_data(void) {
    return ram_data;
}

void set_ram_data(unsigned char data) {
    ram_data = data;
}

void set_ram_oe_pin(int oe) {
    ram_pin_status.oe = oe;
}

void set_ram_we_pin(int we) {
    ram_pin_status.we = we;
}

void set_ram_ce_pin(int ce) {
    ram_pin_status.ce = ce;
    //let ram i/o on the bus.
    if (ce)
        sem_post(&ram_sem_id);
}

static void *ram_loop(void* arg) {
    //ram data load delay is 1/10 (dummy interval)
    struct timespec ts = {CPU_CLOCK_SEC, CPU_CLOCK_NSEC / 10};

    while (!ram_end_loop) {
        sem_wait(&ram_sem_id);
        if (ram_pin_status.ce) {
            if (ram_pin_status.oe) {
                //read cycle
                nanosleep(&ts, NULL);
                ram_data = ram_buffer[ram_addr];
            }
            else if (ram_pin_status.we) {
                //write cycle
                nanosleep(&ts, NULL);
                ram_buffer[ram_addr] = ram_data;
            }
            release_bus();
        }
    }
    return NULL;
}

int init_ram(void) {
    int ret;
    pthread_attr_t attr;
    //struct sched_param sched;

    ram_buffer = malloc(RAM_2K);
    if (!ram_buffer)
        return FALSE;

    ram_addr = 0;
    ram_data = 0;
    ram_pin_status.oe = 0;
    ram_pin_status.we = 0;
    ram_pin_status.ce = 0;

    ram_end_loop = FALSE;

    ret = sem_init(&ram_sem_id, 0, 0);
    if (ret != RT_OK) {
        free(ram_buffer);
        return FALSE;
    }

    ret = pthread_attr_init(&attr);
    if (ret != RT_OK) {
        free(ram_buffer);
        return FALSE;
    }

#if 0
    dprint("priority min:%d, max:%d\n", 
            sched_get_priority_min(SCHED_OTHER), sched_get_priority_max(SCHED_OTHER));
    sched.sched_priority = 0;
    ret = pthread_attr_setschedparam(&attr, &sched);
    if (ret != RT_OK) {
        free(ram_buffer);
        return FALSE;
    }
#endif

    ram_thread_id = 0;
    ret = pthread_create(&ram_thread_id, &attr, ram_loop, NULL);
    if (ret != RT_OK) {
        free(ram_buffer);
        return FALSE;
    }

    return TRUE;
}

void clean_ram(void) {
    void* ret;
    ram_end_loop = TRUE;
    //join the running thread.
    sem_post(&ram_sem_id);
    pthread_join(ram_thread_id, &ret);

    sem_destroy(&ram_sem_id);
    dprint("ram thread joined.\n");

    if (ram_buffer)
        free(ram_buffer);
}

