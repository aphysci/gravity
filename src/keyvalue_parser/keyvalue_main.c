#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>


#include "keyvalue_parser.h"
    
#define log(str, args...) fprintf(stdout, str, ##args )
#define errlog(str, args...) fprintf(stderr, str, ##args )

void usage( const char *me )
{   
    errlog("usage: %s --<option>=<value>\n"
                    "\t--fn=<config file>:\t\tconfiguration file\n"
                    "\t--componentid=<id>:\t\tcomponent ID\n"
                    "\t--help:\t\t\tthis\n",
            me);
    exit(-1);
}

int main(int argc, char *argv[])
{
    
    const char **ppkeys;
    keyvalue_handle_t kv_handle;
    const char *componentid = NULL, *fn = NULL;
    int c, lval, option_index;
    
    struct option long_options[] = 
    {
        { "fn"         ,  required_argument,  &lval,  1 },
        { "componentid",  required_argument,  &lval,  2 },
        { "help",         no_argument,        &lval,  3 },
        { 0,              0,                  0,      0 }
    };
    
    while (1)
    {
        if ( -1 == ( 
                c = getopt_long( argc, argv, "", long_options, &option_index ) ) )
            break;
        
        switch(c)
        {
            case 0:
                if ( 1 == lval )
                {
                    fn = optarg;
                    log("\t%s = %s\n", long_options[option_index].name,
                                        fn);
                }
                else if ( 2 == lval )
                {
                    componentid = optarg;
                    log("\t%s = %s\n", long_options[option_index].name,
                                        componentid);
                }
                else
                    usage(argv[0]);
                lval = -1;
                break;
            default:
                usage(argv[0]);
        }
    }
    
    if ( !fn )
        usage(argv[0]);
    
    {
        const char *sections[3] = { "general", componentid, NULL };
    
        /* Get the KV handle */
        if ( NULL == ( kv_handle = keyvalue_open( fn, sections ) ) )
        {
            errlog("error opening kvhandle\n");
            return -1;
        }
    }
    
    /* Iterate through the keys */
    if ( ( ppkeys = keyvalue_getkeys( kv_handle ) ) )
    {
        const char **_ppkeys;
        for ( _ppkeys = ppkeys; *_ppkeys; _ppkeys++)
            log("%s = %s\n", *_ppkeys, keyvalue_getstring( kv_handle, *_ppkeys ) );
        free( ppkeys );
    }
    
    /* Close the handle */
    keyvalue_close( kv_handle );
    
	return 0;
}
