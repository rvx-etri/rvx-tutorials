#ifndef __TEXPAR_API_H__
#define __TEXPAR_API_H__

#include "texpar_list.h"

typedef struct{
    char *key;
    char *val;
    int used;
} texpar_kvp_t;

typedef struct{
    char *type;
    texpar_list_t *option_list;
} texpar_section_t;

#ifdef __cplusplus
extern "C" {
#endif

texpar_list_t *texpar_read_file_with_section(const char *filename);
texpar_list_t *texpar_read_file_wo_section(const char *filename);

int texpar_read_option(char *s, texpar_list_t *option_list);
void texpar_insert(texpar_list_t *l, char *key, char *val);
char *texpar_find(texpar_list_t *l, char *key);
char *texpar_find_str(texpar_list_t *l, char *key, char *def);
char *texpar_find_str_quiet(texpar_list_t *l, char *key, char *def);
int texpar_find_int(texpar_list_t *l, char *key, int def);
int texpar_find_int_quiet(texpar_list_t *l, char *key, int def);
float texpar_find_float(texpar_list_t *l, char *key, float def);
float texpar_find_float_quiet(texpar_list_t *l, char *key, float def);
void texpar_unused(texpar_list_t *l);
void texpar_free_section(texpar_section_t *s);

//typedef struct {
//	int classes;
//	char **names;
//} metadata;

//LIB_API metadata get_metadata(char *file);

#ifdef __cplusplus
}
#endif
#endif
