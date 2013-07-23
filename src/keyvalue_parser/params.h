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
unsigned int get_bucket_idx( const char *key );

/* Must be a pow2
 */
#define NBUCKETS 64U

#endif
