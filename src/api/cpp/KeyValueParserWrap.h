#ifndef _KEYVALUE_PARSERWRAP_H
#define _KEYVALUE_PARSERWRAP_H

#include <string>
#include <map>
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
    KeyValueConfigParser() {}

    /**
     * Opens the config file and reads in the contents.
     */
    bool Open(const char* filename, std::vector<const char *> &sections) 
    {
        return ( keyvalue_handle = keyvalue_open( filename, &sections[0] ) ) ? true : false;
    }

    /**
     * Frees the resources asscociated with the config file.  Same as the destructor.
     */
    void Close()
    {
        keyvalue_close( keyvalue_handle );
        keyvalue_handle = NULL;
    }

    /**
     * Destructor: closes the config file.
     */
    virtual ~KeyValueConfigParser() 
    {
        Close();
    }

     /**
     * GetString: fetches a string given a key
     */
    std::string GetString(const std::string key, const std::string default_value = "") 
    {
    	std::string key_lower = gravity::StringCopyToLowerCase(key);
        std::string value = keyvalue_getstring( keyvalue_handle, key.c_str() );
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
        const char** keys = keyvalue_getkeys( keyvalue_handle );
        for ( const char **_keys = keys; _keys && *_keys; _keys++ )
            allkeys.push_back( *_keys );
        if (keys)
            free(keys);
        return allkeys;
    }
    
protected:
    keyvalue_handle_t keyvalue_handle;
};

#endif //_KEYVALUE_PARSERWRAP_H
