#ifndef __mapper_h__
#define __mapper_h__


#define FNAME_LEN 1024
extern int mapper_load;
extern char mapper_fname [FNAME_LEN];

int init_mapper(void);
void clean_mapper(void);

#ifdef __mapper_impl__

/*mapper side.*/
int mp_init(void);
int mp_load(void);
void mp_clean(void);

#else /*__mapper_impl__*/

/*emulator side.*/


typedef int (*mp_init_t)(void);
typedef int (*mp_load_t)(void);
typedef void (*mp_clean_t)(void);

mp_init_t mp_init;
mp_load_t mp_load;
mp_clean_t mp_clean;

#endif /*__mapper_impl__*/

#endif /*__mapper_h__*/
