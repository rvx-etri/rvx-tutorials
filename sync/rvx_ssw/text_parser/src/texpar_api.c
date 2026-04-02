//#include <stdio.h>
#include <string.h>

#include "ervp_malloc.h"
#include "ervp_printf.h"
#include "ervp_assert.h"
#include "ervp_fakefile.h"
#include "ervp_stdlib.h"

#include "texpar_utils.h"
#include "texpar_api.h"

texpar_list_t *texpar_read_file_with_section(const char *filename)
{
    FAKEFILE *file = ffopen(filename, "r");
    if(file == 0) file_error(filename);
    char *line;
    int nu = 0;
    texpar_list_t *sections = make_list();
    texpar_section_t *current = 0;
    while((line=ffgetline(file)) != 0){
        ++ nu;
        texpar_strip(line);
        switch(line[0]){
            case '[':
                current = (texpar_section_t*)malloc(sizeof(texpar_section_t));
                assert(current);
                list_insert(sections, current);
                current->option_list = make_list();
                current->type = line;
                break;
            case '\0':
            case '#':
            case ';':
                free(line);
                break;
            default:
                if(!texpar_read_option(line, current->option_list)){
                    assert_msg(0, "Config file error line %d, could parse: %s\n", nu, line);
                    free(line);
                }
                break;
        }
    }
    ffclose(file);
    return sections;
}

texpar_list_t *texpar_read_file_wo_section(const char *filename)
{
    FAKEFILE *file = ffopen(filename, "r");
    if(file == 0) file_error(filename);
    char *line;
    int nu = 0;
    texpar_list_t *option_list = make_list();
    while((line=ffgetline(file)) != 0){
        ++nu;
        texpar_strip(line);
        switch(line[0]){
            case '\0':
            case '#':
            case ';':
                free(line);
                break;
            default:
                if(!texpar_read_option(line, option_list)){
                    assert_msg(0, "Config file error line %d, could parse: %s\n", nu, line);
                    free(line);
                }
                break;
        }
    }
    ffclose(file);
    return option_list;
}

int texpar_read_option(char *s, texpar_list_t *option_list)
{
    size_t i;
    size_t len = strlen(s);
    char *val = 0;
    for(i = 0; i < len; ++i){
        if(s[i] == '='){
            s[i] = '\0';
            val = s+i+1;
            break;
        }
    }
    if(i == len-1) return 0;
    char *key = s;
    texpar_insert(option_list, key, val);
    return 1;
}

void texpar_insert(texpar_list_t *l, char *key, char *val)
{
    texpar_kvp_t* p = (texpar_kvp_t*)malloc(sizeof(texpar_kvp_t));
    assert(p);
    p->key = key;
    p->val = val;
    p->used = 0;
    list_insert(l, p);
}

void texpar_unused(texpar_list_t *l)
{
    texpar_iter_t *n = l->front;
    while(n){
        texpar_kvp_t *p = (texpar_kvp_t *)n->val;
        if(!p->used){
            printf("Unused field: '%s = %s'\n", p->key, p->val);
        }
        n = n->next;
    }
}

char *texpar_find(texpar_list_t *l, char *key)
{
    texpar_iter_t *n = l->front;
    while(n){
        texpar_kvp_t *p = (texpar_kvp_t *)n->val;
        if(strcmp(p->key, key) == 0){
            p->used = 1;
            return p->val;
        }
        n = n->next;
    }
    return 0;
}
char *texpar_find_str(texpar_list_t *l, char *key, char *def)
{
    char *v = texpar_find(l, key);
    if(v) return v;
    if(def) printf("\n%s: Using default '%s'\n", key, def);
    return def;
}

char *texpar_find_str_quiet(texpar_list_t *l, char *key, char *def)
{
    char *v = texpar_find(l, key);
    if (v) return v;
    return def;
}

int texpar_find_int(texpar_list_t *l, char *key, int def)
{
    char *v = texpar_find(l, key);
    if(v) return atoi(v);
    printf("\n%s: Using default '%d'\n", key, def);
    return def;
}

int texpar_find_int_quiet(texpar_list_t *l, char *key, int def)
{
    char *v = texpar_find(l, key);
    if(v) return atoi(v);
    return def;
}

float texpar_find_float_quiet(texpar_list_t *l, char *key, float def)
{
    char *v = texpar_find(l, key);
    if(v) return atof(v);
    return def;
}

float texpar_find_float(texpar_list_t *l, char *key, float def)
{
    char *v = texpar_find(l, key);
    if(v) return atof(v);
    printf("\n%s: Using default '%lf'\n", key, def);
    return def;
}

void texpar_free_section(texpar_section_t *s)
{
    free(s->type);
    texpar_iter_t *n = s->option_list->front;
    while(n){
        texpar_kvp_t *pair = (texpar_kvp_t *)n->val;
        free(pair->key);
        free(pair);
        texpar_iter_t *next = n->next;
        free(n);
        n = next;
    }
    free(s->option_list);
    free(s);
}

