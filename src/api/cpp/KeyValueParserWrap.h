#ifndef _KEYVALUE_PARSERWRAP_H
#define _KEYVALUE_PARSERWRAP_H

#include <string>
#include <map>
#ifndef __GNUC__
#include <memory>
#else
#include <tr1/memory>
#endif

#include "keyvalue_parser.h"


/**
 * A really simple C++ wrapper for the keyvalue library.
 */
class KeyValueConfigParser
{
public:
    /**
     * Constructor
     */
    KeyValueConfigParser(const char* filename, const std::vector<const char *> &sections) :
        spKeyValueHandle( keyvalue_open( filename, &sections[0] ), keyvalue_close ) {}

     /**
     * GetString: fetches a string given a key
     */
    std::string GetString(const std::string key, const std::string default_value = "") 
    {
    	std::string key_lower = gravity::StringCopyToLowerCase(key);
        /* Returns "" if not found */
        std::string value( keyvalue_getstring( spKeyValueHandle.get(), key.c_str() ) );
    	if ( value.length() )
            return value;
        return default_value;
    }

     /**
     * GetKeys: fetches all the keys
     */
    std::vector<std::string> GetKeys() 
    {
        std::vector<std::string> allkeys;
        const char** keys = keyvalue_getkeys( spKeyValueHandle.get() );
        for ( const char **_keys = keys; _keys && *_keys; _keys++ )
            allkeys.push_back( *_keys );
        if (keys)
            free(keys);
        return allkeys;
    }
    
protected:
    std::tr1::shared_ptr< keyvalue_type_t > spKeyValueHandle;
};

#endif //_KEYVALUE_PARSERWRAP_H
