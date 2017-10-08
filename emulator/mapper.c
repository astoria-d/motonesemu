#include <dlfcn.h>
#include <stdio.h>

#include "tools.h"
#include "mapper.h"

void set_rom_addr(unsigned short addr);
unsigned char get_rom_data(void);


int mapper_load;
char mapper_fname [FNAME_LEN];
mp_set_addr_t mp_set_addr;
mp_get_data_t mp_get_data;


static void* mapper_dl;

int init_mapper(void) {
    mapper_dl = NULL;
    mp_set_addr = NULL;
    mp_get_data = NULL;

    if (mapper_load == TRUE) {
        if((mapper_dl = dlopen(mapper_fname, RTLD_LAZY)) == NULL) {
            printf("%s\n", dlerror());
            return FALSE;
        }
        if((mp_init = dlsym(mapper_dl , "mp_init")) == NULL) {
            printf("%s\n", dlerror());
            return FALSE;
        }
        
        if((mp_set_addr = dlsym(mapper_dl , "mp_set_addr")) == NULL) {
            printf("%s\n", dlerror());
            return FALSE;
        }

        if((mp_get_data = dlsym(mapper_dl , "mp_get_data")) == NULL) {
            printf("%s\n", dlerror());
            return FALSE;
        }
        /*mp_set_debugger is optional..*/
        mp_set_debugger = dlsym(mapper_dl , "mp_set_debugger");
        mp_dbg_get_byte = dlsym(mapper_dl , "mp_dbg_get_byte");
        mp_dbg_get_short = dlsym(mapper_dl , "mp_dbg_get_short");

        (*mp_init)(set_rom_addr, get_rom_data);
    }
    return TRUE;
}

void clean_mapper(void) {
    if (mapper_load == TRUE) {
        if((mp_clean = dlsym(mapper_dl , "mp_clean")) == NULL) {
            printf("%s\n", dlerror());
            return;
        }
        (*mp_clean)();
    }
}

/*
load_memory
    set_bus_addr
        set_rom_addr
        set_apu_addr
        set_ppu_addr
        set_ram_addr
    start_bus
    get_bus_data
        get_rom_data
        get_apu_data
        get_ppu_data
        get_ram_data
    end_bus
    

store_memory
    set_bus_addr
        set_rom_addr
        set_apu_addr
        set_ppu_addr
        set_ram_addr
    set_bus_data
        set_apu_data
        set_ppu_data
        set_ram_data
    start_bus
    end_bus



*/
