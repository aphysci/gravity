#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#ifndef _WIN32
#include <getopt.h>
#endif
#include <errno.h>
#include <string.h>


#include "keyvalue_parser.h"

#define log(str, ...) fprintf(stdout, str, ##__VA_ARGS__ )
#define errlog(str, ...) fprintf(stderr, str, ##__VA_ARGS__ )

void usage( const char *me )
{   
#ifdef _WIN32
    errlog("usage: %s <config_file> <componentid>\n"
                    "\t<config file>:\t\tconfiguration file\n"
                    "\t<componentid>:\t\tcomponent ID\n",
            me);
#else
    errlog("usage: %s --<option>=<value>\n"
                    "\t--fn=<config file>:\t\tconfiguration file\n"
                    "\t--componentid=<id>:\t\tcomponent ID\n"
                    "\t--help:\t\t\tthis\n",
            me);
#endif
    exit(-1);
}

int main(int argc, char *argv[])
{
    
    const char **ppkeys;
    keyvalue_handle_t kv_handle;
    const char *componentid = NULL, *fn = NULL;
#ifndef _WIN32
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
#else
	fn = argv[1];
	componentid = argv[2];
#endif

    
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
        free( (char**)ppkeys );
    }
    
    /* Close the handle */
    keyvalue_close( kv_handle );
    
	return 0;
}
