#include <string.h>

#include "ervp_assert.h"
#include "ervp_malloc.h"

#include "texpar_list.h"
#include "texpar_api.h"

texpar_list_t *make_list()
{
    texpar_list_t* l = (texpar_list_t*)malloc(sizeof(texpar_list_t));
    assert(l);
    l->size = 0;
    l->front = 0;
    l->back = 0;
    return l;
}

/*
void transfer_node(texpar_list_t *s, texpar_list_t *d, texpar_iter_t *n)
{
    texpar_iter_t *prev, *next;
    prev = n->prev;
    next = n->next;
    if(prev) prev->next = next;
    if(next) next->prev = prev;
    --s->size;
    if(s->front == n) s->front = next;
    if(s->back == n) s->back = prev;
}
*/

void *list_pop(texpar_list_t *l){
    if(!l->back) return 0;
    texpar_iter_t *b = l->back;
    void *val = b->val;
    l->back = b->prev;
    if(l->back) l->back->next = 0;
    free(b);
    --l->size;

    return val;
}

void list_insert(texpar_list_t *l, void *val)
{
    texpar_iter_t* newnode = (texpar_iter_t*)malloc(sizeof(texpar_iter_t));
    assert(newnode);
    newnode->val = val;
    newnode->next = 0;

    if(!l->back){
        l->front = newnode;
        newnode->prev = 0;
    }else{
        l->back->next = newnode;
        newnode->prev = l->back;
    }
    l->back = newnode;
    ++l->size;
}

void free_node(texpar_iter_t *n)
{
    texpar_iter_t *next;
    while(n) {
        next = n->next;
        free(n);
        n = next;
    }
}

void free_list_val(texpar_list_t *l)
{
    texpar_iter_t *n = l->front;
    texpar_iter_t *next;
    while (n) {
        next = n->next;
        free(n->val);
        n = next;
    }
}

void free_list(texpar_list_t *l)
{
    free_node(l->front);
    free(l);
}

void free_list_contents(texpar_list_t *l)
{
    texpar_iter_t *n = l->front;
    while(n){
        free(n->val);
        n = n->next;
    }
}

void free_list_contents_kvp(texpar_list_t *l)
{
    texpar_iter_t *n = l->front;
    while (n) {
        texpar_kvp_t* p = (texpar_kvp_t*)n->val;
        free(p->key);
        free(n->val);
        n = n->next;
    }
}

void **list_to_array(texpar_list_t *l)
{
    void** a = (void**)calloc(l->size, sizeof(void*));
    int count = 0;
    texpar_iter_t *n = l->front;
    while(n){
        a[count++] = n->val;
        n = n->next;
    }
    return a;
}
