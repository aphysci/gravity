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
  
var_list_t g_var_list;
const char *g_section_name;
extern char *g_current_section;

extern int yyparse();
extern int yydebug;


int yywrap(void)
{ return 1; }


int keyvalue_parse_params(const char* param_fn, const char *section_list[], var_list_t **ppvarlist)
{
    const char **_section_list;
    int retval = 1;
    yydebug = 0;
    g_var_list.len = 0;
    g_var_list.head = NULL;
    g_var_list.tail = NULL;
    
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
            retval = yyparse();
            yy_delete_buffer( YY_CURRENT_BUFFER );
            fclose(yyin);
            if ( retval ) 
                break;
        }
    }
    if ( retval || !ppvarlist )
        free_var_list(&g_var_list);
    else 
        *ppvarlist = &g_var_list;
    
    return retval;
}

%}
%x comment
%option yylineno
%option nounistd
%option never-interactive
%%

[+()=*\-\n\/]                                           { return(*yytext); }
<INITIAL>#	                                            { BEGIN(comment); }
<comment>[^\r\n]*                                       { }
<comment>(\n)                                           { yyless(1); BEGIN(INITIAL);return('\n'); }
[A-Za-z0-9\_\.\@;:]+                                    { yylval = yytext; return(TOKEN); }
\"[ ]*[-A-Za-z0-9\_\.\@\/;\":\$+()=\\*\-\ ]*[ ]*\"      { yylval = yytext; return(TOKEN_STRING); }
\[[ ]*[A-Za-z0-9\_\.\@\/;]+[ ]*\]                       { yylval = yytext; return(SECTION); }
$[A-Za-z0-9\_\.\@;]+                                    { yylval = yytext+1; return(VAR); }
[ \t]+                                                  { }
.                                                       { printf("%s ", yytext ); yyerror("invalid character"); return(ERROR);}
