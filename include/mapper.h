#ifndef __mapper_h__
#define __mapper_h__

typedef void (*mp_set_addr_t)(unsigned short addr);
typedef unsigned char (*mp_get_data_t)(void);

typedef unsigned char (*mp_dbg_get_byte_t)(unsigned short offset);
typedef unsigned short (*mp_dbg_get_short_t)(unsigned short offset);


#ifdef __mapper_impl__

/*======================
     mapper side.
======================*/
int mp_init(mp_set_addr_t set_func, mp_get_data_t get_func);
int mp_set_debugger(mp_dbg_get_byte_t byte_func, mp_dbg_get_short_t short_func);
void mp_clean(void);

void mp_set_addr(unsigned short addr);
unsigned char mp_get_data(void);

unsigned char mp_dbg_get_byte(unsigned short offset);
unsigned short mp_dbg_get_short(unsigned short offset);

#else /*__mapper_impl__*/

/*======================
    emulator side.
======================*/

#define FNAME_LEN 1024
extern int mapper_load;
extern char mapper_fname [FNAME_LEN];

int init_mapper(void);
void clean_mapper(void);


typedef int (*mp_init_t)(mp_set_addr_t set_func, mp_get_data_t get_func);
typedef void (*mp_clean_t)(void);
typedef int (*mp_set_debugger_t)(mp_dbg_get_byte_t byte_func, mp_dbg_get_short_t short_func);

mp_init_t mp_init;
mp_clean_t mp_clean;

mp_set_addr_t mp_set_addr;
mp_get_data_t mp_get_data;

mp_set_debugger_t mp_set_debugger;
mp_dbg_get_byte_t mp_dbg_get_byte;
mp_dbg_get_short_t mp_dbg_get_short;

#endif /*__mapper_impl__*/

#endif /*__mapper_h__*/
