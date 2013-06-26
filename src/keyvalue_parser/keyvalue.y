%{

/* keyvalue parser specification
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "params.h"

#define YYSTYPE char*
#define YYERROR_VERBOSE

extern int yylineno;
void yyerror(const char *s)
{
	fprintf(stderr, "yacc: error line %d: %s\n", yylineno, s);
}
extern char* yytext;
extern var_list_t g_var_list;
char *g_current_section = NULL;
extern const char *g_section_name;
#define CHECK_SECTION ( g_current_section && 0 == strcasecmp( g_current_section, g_section_name ) )

%}
%start file
%token TOKEN SECTION VAR ERROR TOKEN_STRING
%left '*' '/'
%left '-' '+'
%left '='
//%expect 10
%%

file: kvpair '\n'
    | file kvpair '\n'
    | file kvpair
    | section '\n'
    | section
    | file section '\n'
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
            var_list_insert(&g_var_list, var_el);
        }
    } 

expr:
    ERROR
    {
        yyerror("parse error");
        YYABORT;
    }
    | VAR
    {
        if ( CHECK_SECTION )
        {
            var_el_t *var_el = (var_el_t*)var_list_find(&g_var_list, (char*)$1); 
            if ( !var_el )
            {
                char str[32];
                snprintf(str, sizeof(str), "unknown variable %s", $1);
                yyerror(str);
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




