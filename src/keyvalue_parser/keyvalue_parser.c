#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    for (var_el = var_list_first(pvar_list); key && var_el; var_el = var_el->next )
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
    pvar_el_t var_el;
    while ( ( var_el = var_list_first( pvar_list ) ) )
    {
        free(var_el->key);
        free(var_el->val);
        var_list_remove( pvar_list, var_el );
        free(var_el);
    }
}

/* Get the length of the parameter list
 */
int var_list_length( pvar_list_t pvar_list )
{
    if ( pvar_list )
        return pvar_list->len;
    return 0;
}

/* Get the first element in a var list
 */
pvar_el_t var_list_first( pvar_list_t pvar_list )
{
    if ( pvar_list )
        return pvar_list->head;
    return NULL;
}

/* Remove an element from the list
 */
void var_list_remove( pvar_list_t pvar_list, pvar_el_t pvar_el )
{
    if ( !pvar_list || !pvar_el )
        return;
    if ( pvar_el->next )
        pvar_el->next->prev = pvar_el->prev;
    if ( pvar_el->prev )
        pvar_el->prev->next = pvar_el->next;
    
    if ( pvar_list->head == pvar_el )
        pvar_list->head = pvar_el->next;
    
    pvar_el->next = NULL;
    pvar_el->prev = NULL;
    pvar_list->len--;
}

/* Insert an element at the head of the list
 */
void var_list_insert( pvar_list_t pvar_list, pvar_el_t pvar_el )
{
    if ( !pvar_list || !pvar_el )
        return;
    pvar_el->next = pvar_list->head;
    pvar_el->prev = NULL;
    if ( pvar_list->head )
        pvar_list->head->prev = pvar_el;
    pvar_list->head = pvar_el;
    pvar_list->len++;
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
        (var_list_length((pvar_list_t)kv_handle ) + 1 ) * sizeof(const char *));
    if (NULL == ppkeys) 
        return ppkeys;
    for (i = 0, var_el = var_list_first((pvar_list_t)kv_handle); var_el; 
            var_el = var_el->next, i++ )
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





