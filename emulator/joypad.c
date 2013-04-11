#include <pthread.h>
#include <string.h>

#include "tools.h"

int init_joypad_wnd(void);
void* window_start(void* arg);
void close_joypad_wnd(void);

int get_button(unsigned int code);

struct read_joypad_reg {
    unsigned int button_data    :1;
    unsigned int ext_data       :2;
    unsigned int zapper_trigger :1;
    unsigned int zapper_sprite  :1;
    unsigned int nouse          :3;
};

struct write_joypad_reg {
    unsigned int button_set     :1;
    unsigned int ext_data       :2;
    unsigned int nouse          :5;
};

struct joypad_reg {
    struct write_joypad_reg w;
    struct read_joypad_reg r;
    unsigned int read_cnt    :3;
};

static struct joypad_reg jp_reg;

static pthread_t jp_thread_id;

void set_joypad_data(unsigned char data) {
    struct write_joypad_reg old;
    //dprint("set joypad data:%x\n", data);

    old = jp_reg.w;
    memcpy(&jp_reg.w, &data, sizeof(unsigned char));
    if (jp_reg.w.button_set == 0 && old.button_set) {
        jp_reg.read_cnt = 0;
    }
}

unsigned char get_joypad_data(void) {
    unsigned char data;
    jp_reg.r.button_data = get_button(jp_reg.read_cnt++);
    memcpy(&data, &jp_reg.w, sizeof(unsigned char));
    //dprint("get joypad data(%d) %d\n", jp_reg.read_cnt, data);
    return data;
}

int init_joypad(void) {
    int ret;
    pthread_attr_t attr;

    memset(&jp_reg, sizeof(struct joypad_reg), 0);

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


