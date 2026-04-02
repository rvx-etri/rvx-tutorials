#ifndef __TEXPAR_LIST_H__
#define __TEXPAR_LIST_H__

typedef struct texpar_iter{
    void *val;
    struct texpar_iter *next;
    struct texpar_iter *prev;
} texpar_iter_t;

typedef struct texpar_list{
    int size;
    texpar_iter_t *front;
    texpar_iter_t *back;
} texpar_list_t;

#ifdef __cplusplus
extern "C" {
#endif
texpar_list_t *make_list();
int list_find(texpar_list_t *l, void *val);

void list_insert(texpar_list_t *, void *);

void **list_to_array(texpar_list_t *l);

void free_list_val(texpar_list_t *l);
void free_list(texpar_list_t *l);
void free_list_contents(texpar_list_t *l);
void free_list_contents_kvp(texpar_list_t *l);

#ifdef __cplusplus
}
#endif
#endif
