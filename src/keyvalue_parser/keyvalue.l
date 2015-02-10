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


%{

/* keyvalue - lexer specification
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "params.h"

#define YYSTYPE char*
#include "y.tab.h"
extern YYSTYPE yylval;
  
var_list_t g_buckets[ NBUCKETS ] ;
const char *g_section_name;
extern char *g_current_section;

extern int yyparse(void* pbuckets);
extern void yyerror(const char *s);
extern int yydebug;


int yywrap(void)
{ return 1; }


int keyvalue_parse_params(const char* param_fn, const char * const section_list[], var_list_t **ppvarlist)
{
    const char * const *_section_list;
    int retval = 1, i;
    yydebug = 0;
    for ( i = 0; i < NBUCKETS; i++ )
    {
        g_buckets[i].len = 0;
        g_buckets[i].head = NULL;
        g_buckets[i].tail = NULL;
    }
    
    if ( ppvarlist )
        *ppvarlist = NULL;
    
    for ( _section_list = section_list; _section_list && *_section_list; _section_list++)
    {
        g_section_name = *_section_list;
        yyin = fopen(param_fn, "r");
        yylineno = 1;
        if (yyin)
        {
            yy_switch_to_buffer(yy_create_buffer( yyin, YY_BUF_SIZE ) );
            retval = yyparse(g_buckets);
            yy_delete_buffer( YY_CURRENT_BUFFER );
            fclose(yyin);
            if ( retval ) 
                break;
        }
    }
    if ( retval || !ppvarlist )
        free_var_list(g_buckets);
    else 
        *ppvarlist = g_buckets;
    
    return retval;
}

%}
%x comment
%option yylineno
%option nounistd
%option never-interactive
%%

[+()=*\-\n\r\/]                                           { return(*yytext); }
<INITIAL>#	                                            { BEGIN(comment); }
<comment>[^\r\n]*                                       { }
<comment>(\n)                                           { yyless(1); BEGIN(INITIAL);return('\n'); }
[A-Za-z0-9\_\.\@;:,]+                                    { yylval = yytext; return(TOKEN); }
\"[ ]*[-A-Za-z0-9\_\.\@\/;,\":\$+()=\\*\-\ ]*[ ]*\"      { yylval = yytext; return(TOKEN_STRING); }
\[[ ]*[A-Za-z0-9\_\.\@\/;]+[ ]*\]                       { yylval = yytext; return(SECTION); }
$[A-Za-z0-9\_\.\@;]+                                    { yylval = yytext+1; return(VAR); }
[ \t]+                                                  { }
.                                                       { printf("%s ", yytext ); yyerror("invalid character"); return(ERROR);}
