#ifndef _KEYVALPARAMS_H
#define _KEYVALPARAMS_H

typedef struct _var_el
{
    char *key,
         *val;
    struct _var_el *next, *prev;
} var_el_t, *pvar_el_t;

typedef struct _var_list
{
    struct _var_el *head, *tail;
    int len;
} var_list_t, *pvar_list_t;

var_el_t *var_list_find(pvar_list_t pvar_list, const char* key );
void free_var_list(pvar_list_t pvar_list);
int var_list_length( pvar_list_t pvar_list );
pvar_el_t var_list_first( pvar_list_t pvar_list );
void var_list_remove( pvar_list_t pvar_list, pvar_el_t pvar_el );
void var_list_insert( pvar_list_t pvar_list, pvar_el_t pvar_el );

#endif
