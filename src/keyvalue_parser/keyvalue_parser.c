/** (C) Copyright 2013, Applied Physical Sciences Corp., A General Dynamics Company
 **
 ** Gravity is free software; you can redistribute it and/or modify
 ** it under the terms of the GNU Lesser General Public License as published by
 ** the Free Software Foundation; either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU Lesser General Public License for more details.
 **
 ** You should have received a copy of the GNU Lesser General Public
 ** License along with this program;
 ** If not, see <http://www.gnu.org/licenses/>.
 **
 */

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
int keyvalue_parse_params(const char* param_fn, const char* const section_list[], var_list_t **ppvarlist);


/*
 * Hash a given key
 */
unsigned int get_bucket_idx( const char *key )
{
    unsigned int val = 0;
    for(; key && *key; key++)
        val += (unsigned int)*key;
    return val & ( NBUCKETS - 1 );
}


/*
 * Find a parameter by key
 */
var_el_t *var_list_find(pvar_list_t pvar_list, const char* key )
{
    pvar_el_t var_el;
    for (var_el = var_list_first(pvar_list); key && var_el; var_el = var_el->next )
    {
        if (0 == strncmp(var_el->key, key, strlen(key) ) && strlen(var_el->key) == strlen(key) )
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
    int i;
    if ( !pvar_list ) 
        return;
    for ( i = 0; i < NBUCKETS; i++ )
    {
        while ( ( var_el = var_list_first( pvar_list + i ) ) )
        {
            free(var_el->key);
            free(var_el->val);
            var_list_remove( pvar_list + i, var_el );
            free(var_el);
        }
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
    if ( pvar_list->tail == pvar_el )
        pvar_list->tail = pvar_el->prev;
    
    pvar_el->next = NULL;
    pvar_el->prev = NULL;
    pvar_list->len--;
}

/* Insert an element at the tail of the list in this key's bucket
 */
void var_list_insert( pvar_list_t pvar_list, pvar_el_t pvar_el )
{
    if ( !pvar_list || !pvar_el )
        return;
    pvar_el->prev = pvar_list->tail;
    pvar_el->next = NULL;
    if ( pvar_list->tail )
        pvar_list->tail->next = pvar_el;
    pvar_list->tail = pvar_el;
    if ( !pvar_list->head )
        pvar_list->head = pvar_el;
    pvar_list->len++;
}

/* 
 * Public Methods
 ***************************************************/

/* Open a keyvalue file and return a handle to it 
 */
keyvalue_handle_t keyvalue_open(const char *fn, const char* const sections[] )
{
    pvar_list_t pvar_list = NULL;
    keyvalue_parse_params( fn, sections, &pvar_list );
    return (keyvalue_handle_t)pvar_list;
}

/* Get all the keys parsed
 */
const char **keyvalue_getkeys(keyvalue_handle_t kv_handle )
{
    pvar_el_t var_el;
    int i, j = 0, nentries = 0;
    const char **ppkeys;
    if ( kv_handle )
    {
        for ( i = 0; i < NBUCKETS; i++ )
            nentries += var_list_length((pvar_list_t)kv_handle + i);
    }
    
    if ( NULL == ( ppkeys = malloc( ( nentries + 1 ) * sizeof(const char *) ) ) )
        return ppkeys;
    
    if ( kv_handle )
    {
        for ( i = 0; i < NBUCKETS; i++ )
            for (var_el = var_list_first((pvar_list_t)kv_handle + i); var_el; 
                    var_el = var_el->next, j++ )
                        ppkeys[j] = var_el->key;
    }
    ppkeys[j] = NULL;
    return ppkeys;
}

/* Get a value given a key
 */
const char *keyvalue_getstring(keyvalue_handle_t kv_handle, const char *key)
{
    pvar_el_t var_el;
    if ( kv_handle && ( var_el = var_list_find((pvar_list_t)kv_handle + get_bucket_idx(key), key ) ) )
        return var_el->val;
    return "";
}

/* Close a keyvalue file handle 
 */
void keyvalue_close(keyvalue_handle_t kv_handle)
{
    free_var_list((pvar_list_t)kv_handle );
}



