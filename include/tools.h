#ifndef __tools_h__
#define __tools_h__

struct slist {
    struct slist *next;
} ;

struct dlist {
    struct dlist *prev;
    struct dlist *next;
} ;


void slist_add_tail (struct slist* dest, struct slist* node) ;
int slist_count (struct slist* head);

void dlist_init (struct dlist* node) ;
void dlist_add_next (struct dlist* dest, struct dlist* node) ;
void dlist_add_prev (struct dlist* dest, struct dlist* node) ;
int dlist_remove (struct dlist* node) ;
int dlist_count (struct dlist* head);


#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#define RT_OK 0
#define RT_ERROR -1

#endif /*__tools_h__*/

