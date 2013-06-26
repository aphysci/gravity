#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "queue.h"
#include "keyvalue_parser.h"
#include "params.h"

/* 
 * Private Methods 
 ***************************************************/

/*
 * Defined in the Lexer spec, kicks off the parameter file parsing
 */
int keyvalue_parse_params(const char* param_fn, const char *section_list[], var_list_t **ppvarlist);

/*
 * Find a parameter by key
 */
var_el_t *var_list_find(pvar_list_t pvar_list, const char* key )
{
    pvar_el_t var_el;
    for (var_el = var_list_top(pvar_list); key && var_el; var_el = var_list_next(var_el) )
    {
        if (0 == strncmp(var_el->key, key, strlen(key) ) )
            return var_el;
    }
    return NULL;
}

/*
 * Free parameter list 
 */
void free_var_list(pvar_list_t pvar_list)
{
    pvar_el_t var_el = var_list_top(pvar_list), tvar_el = NULL;
    while (var_list_qlen(pvar_list) )
    {
        free(var_el->key);
        free(var_el->val);
        tvar_el = var_list_remove_inc(pvar_list, var_el);
        free(var_el);
        var_el = tvar_el;
    }
}


/* 
 * Public Methods
 ***************************************************/

/* Open a keyvalue file and return a handle to it 
 */
keyvalue_handle_t keyvalue_open(const char *fn, const char* sections[] )
{
    pvar_list_t pvar_list = NULL;
    keyvalue_parse_params( fn, sections, &pvar_list );
    return (keyvalue_handle_t)pvar_list;
}

/* Get all the keys parsed
 */
const char** keyvalue_getkeys(keyvalue_handle_t kv_handle )
{
    pvar_el_t var_el;
    int i;
    const char **ppkeys = malloc( 
        (var_list_qlen((pvar_list_t)kv_handle ) + 1 ) * sizeof(const char *));
    if (NULL == ppkeys) 
        return ppkeys;
    for (i = 0, var_el = var_list_top((pvar_list_t)kv_handle); var_el; 
            var_el = var_list_next(var_el), i++ )
        ppkeys[i] = var_el->key;
    ppkeys[i] = NULL;
    return ppkeys;
}

/* Get a value given a key
 */
const char *keyvalue_getstring(keyvalue_handle_t kv_handle, const char *key)
{
    pvar_el_t var_el;
    if ( ( var_el = var_list_find((pvar_list_t)kv_handle, key ) ) )
        return var_el->val;
    return NULL;
}

/* Close a keyvalue file handle 
 */
void keyvalue_close(keyvalue_handle_t kv_handle)
{
    free_var_list((pvar_list_t)kv_handle );
}

QUEUE_FUNCTION_INSTANTIATIONS(var_list, ptrs, list, struct _var_el, struct _var_list);


