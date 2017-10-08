#include <dlfcn.h>
#include <stdio.h>

#include "tools.h"
#include "mapper.h"

int mapper_load;
char mapper_fname [FNAME_LEN];


static void* mapper_dl;

int init_mapper(void) {
    mapper_dl = NULL;

    if (mapper_load == TRUE) {
        if((mapper_dl = dlopen(mapper_fname, RTLD_LAZY)) == NULL) {
            printf("%s\n", dlerror());
            return FALSE;
        }
        if((mp_init = dlsym(mapper_dl , "mp_init")) == NULL) {
            printf("%s\n", dlerror());
            return FALSE;
        }
        (*mp_init)();
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

