#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "tools.h"
#include "vga.h"

static int   shmid;

void *vga_shm_get(void) {
    void* ret;
    key_t key;

    //create shared memory
    key = ftok(VGA_SHM, VGA_SHM_PRJ_ID);
    if (key == -1) {
        fprintf(stderr, "error preparing shared memory.\n");
        return NULL;
    }

    if((shmid = shmget(key, VGA_SHM_SIZE, IPC_CREAT|IPC_EXCL|0666)) == -1) 
    {
        //printf("Shared memory segment exists - opening as client\n");

        /* Segment probably already exists - try as a client */
        if((shmid = shmget(key, VGA_SHM_SIZE, 0)) == -1) 
        {
            fprintf(stderr, "error opening shared memory.\n");
            return NULL;
        }
    }

    /* Attach (map) the shared memory segment into the current process */
    if((ret = shmat(shmid, 0, 0)) == (void*)-1)
    {
        fprintf(stderr, "error attaching shared memory.\n");
        return NULL;
    }

    return ret;
}

void vga_shm_free(void* addr) {
    shmdt(addr);
}

void vga_shm_remove(void) {
    shmctl(shmid, IPC_RMID, 0);
}

