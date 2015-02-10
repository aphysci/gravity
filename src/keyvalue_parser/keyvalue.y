/* (C) Copyright 2013, Applied Physical Sciences Corp., A General Dynamics Company
 *
 * Gravity is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program;
 * If not, see <http://www.gnu.org/licenses/>.
 *
 */

%parse-param {void *pbuckets}
%{

/* keyvalue parser specification
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#include "params.h"

#define YYSTYPE char*
#define YYERROR_VERBOSE
#ifdef WIN32
#   define snprintf _snprintf
#   define strcasecmp stricmp
#endif
extern int yylineno;
void yyerror(void *p, const char *s)
{
	fprintf(stderr, "yacc: error line %d: %s\n", yylineno, s);
}
extern char* yytext;
extern int yylex (void);
char *g_current_section = NULL;
extern const char *g_section_name;
#define CHECK_SECTION ( g_current_section && 0 == strcasecmp( g_current_section, g_section_name ) )
%}
%start file
%token TOKEN SECTION VAR ERROR TOKEN_STRING
%left '*' '/'
%left '-' '+'
%left '='
%expect 19
%%

file: kvpair '\n'
      kvpair '\r' '\n'
    | file kvpair '\n'
    | file kvpair '\r' '\n'
    | file kvpair
    | section '\n'
    | section '\r' '\n'
    | section
    | file section '\n'
    | file section '\r' '\n'
    | file section
;

section:
    SECTION
    {
        char *begin = yytext;
        char *end = &yytext[ strlen(yytext) - 1 ];
        if ( g_current_section)
            free(g_current_section);
        while (begin && !isalnum((int)*begin))
            begin++;
        while (end && !isalnum((int)*end))
            end--;
        if ( end )
            *++end = '\0';
        g_current_section = strdup(begin);
    } | ;

kvpair: 
    expr '=' expr
    { 
        if ( CHECK_SECTION )
        {
            var_el_t *var_el = (var_el_t*)malloc(sizeof(var_el_t) );
            var_el->next = var_el->prev = NULL;
            var_el->key = strdup($1);
            var_el->val = strdup($3);
            free($1); free($3);
            var_list_insert(& ( (pvar_list_t)pbuckets)[ get_bucket_idx( var_el->key ) ], var_el);
        }
    } 

expr:
    ERROR
    {
        yyerror(NULL, "parse error");
        YYABORT;
    }
    | VAR
    {
        if ( CHECK_SECTION )
        {
            var_el_t *var_el = (var_el_t*)var_list_find(& ( (pvar_list_t)pbuckets)[ get_bucket_idx($1) ], (char*)$1); 
            if ( !var_el )
            {
                char str[32];
                snprintf(str, sizeof(str), "unknown variable %s", $1);
                yyerror(NULL, str);
                YYABORT;
            }
            else
                $$ = strdup(var_el->val);
        }
    }          
    | TOKEN 
    {
        if ( CHECK_SECTION )
        {
            $$ = strdup(yytext);
        }
    }
    | TOKEN_STRING 
    {
        if ( CHECK_SECTION )
        {
            yytext[ strlen(yytext) - 1 ] = '\0';
            $$ = strdup(yytext+1);
        }
    }
    | expr '+' expr 
    { 
        if ( CHECK_SECTION )
        {
            char str[32]; 
            snprintf(str, sizeof(str), "%f", atof($1) + atof($3) ); 
            $$ = strdup(str);
        }
    }
    | expr '*' expr 
    { 
        if ( CHECK_SECTION )
        {
            char str[32]; 
            snprintf(str, sizeof(str), "%f", atof($1) * atof($3) ); 
            $$ = strdup(str);
        }
    }
    | expr '-' expr 
    { 
        if ( CHECK_SECTION )
        {
            char str[32]; 
            snprintf(str, sizeof(str), "%f", atof($1) - atof($3) ); 
            $$ = strdup(str);
        }
    }
    |
    expr '/' expr 
    { 
        if ( CHECK_SECTION )
        {
            char str[32]; 
            snprintf(str, sizeof(str), "%f", atof($1) / atof($3) ); 
            $$ = strdup(str);
        }
    }
    | '(' expr ')' 
    { 
        if ( CHECK_SECTION )
        { 
            $$ = $2; 
        } 
    }
;

%%




