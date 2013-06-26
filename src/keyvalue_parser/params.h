#ifndef _KEYVALPARAMS_H
#define _KEYVALPARAMS_H

typedef struct _var_el
{
    char *key,
         *val;
    QUEUE_ELEMENT_DECL(ptrs, struct _var_el);
} var_el_t, *pvar_el_t;

typedef struct _var_list
{
    QUEUE_DECL(list, struct _var_el);
} var_list_t, *pvar_list_t;

QUEUE_INLINE_INSTANTIATIONS(var_list, ptrs, list, struct _var_el, struct _var_list);

var_el_t *var_list_find(pvar_list_t pvar_list, const char* key );
void free_var_list(pvar_list_t pvar_list);

#endif
