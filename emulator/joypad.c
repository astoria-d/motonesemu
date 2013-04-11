#include <pthread.h>

#include "tools.h"

int init_joypad_wnd(void);
void* window_start(void* arg);
void close_joypad_wnd(void);


static pthread_t jp_thread_id;

int init_joypad(void) {
    int ret;
    pthread_attr_t attr;

    ret = init_joypad_wnd();
    if (ret == FALSE) {
        return FALSE;
    }

    ret = pthread_attr_init(&attr);
    if (ret != RT_OK)
        return FALSE;

    jp_thread_id = 0;
    ret = pthread_create(&jp_thread_id, &attr, window_start, NULL);
    if (ret != RT_OK)
        return FALSE;


    return TRUE;
}

void clean_joypad(void) {
    void *ret;
    close_joypad_wnd();
    pthread_join(jp_thread_id, &ret);
}


