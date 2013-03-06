
#include <stdio.h>

#define DPRINT_IMPL

#include "tools.h"

void dprint(char* s, char* arg1, unsigned int arg2, unsigned int arg3, 
        unsigned int arg4, unsigned int arg5, unsigned int arg6) {
     printf(s, arg1, arg2, arg3, arg4, arg5, arg6 );
    /*
     **/
}

void slist_add_tail (struct slist* dest, struct slist* node) {
    struct slist *p = dest;
    struct slist *pp = dest;
    while( (p = p->next) != NULL ) {
        pp = p;
    }
    pp->next = node;
}

int slist_count (struct slist* head) {
    int ret = 0;
    struct slist* p = head;
    while( p != NULL ) {
        p = p->next;
        ret++;
    }
    return ret;
}

void dlist_init (struct dlist* node) {
    node->next = node->prev = NULL;
}

void dlist_add_next (struct dlist* dest, struct dlist* node) {
    struct dlist* next_node = dest->next;

    if (next_node) {
        next_node->prev = node;
    }
    dest->next = node;
    node->prev = dest;
    node->next = next_node;
}

void dlist_add_prev (struct dlist* dest, struct dlist* node) {
    struct dlist* prev_node = dest->prev;

    if (prev_node) {
        prev_node->next = node;
    }
    dest->prev = node;
    node->prev = prev_node;
    node->next = dest;
}

void dlist_add_tail (struct dlist* dest, struct dlist* node) {
    struct dlist* next_node = dest->next;
    struct dlist* tail_node = dest;

    while (next_node != NULL) {
        tail_node = next_node;
        next_node = tail_node->next;
    }
    dlist_add_next(tail_node, node);
}

int dlist_remove (struct dlist* node) {
    struct dlist* next_node = node->next;
    struct dlist* prev_node = node->prev;

    int ret = FALSE;
    if (next_node) {
        ret = TRUE; 
        next_node->prev = prev_node;
    }
    if (prev_node) {
        ret = TRUE; 
        prev_node->next = next_node;
    }
    return ret;
}


int dlist_count (struct dlist* head) {
    int ret = 0;
    struct dlist* p = head;
    while( p != NULL ) {
        p = p->next;
        ret++;
    }
    return ret;
}


