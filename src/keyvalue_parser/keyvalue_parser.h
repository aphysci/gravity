#ifndef __KEYVALUE_PARSER_H
#define __KEYVALUE_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
#   ifdef LIBKEYVALUE_PARSER_EXPORTS
#        define KEYVALUE_API __declspec(dllexport)
#   else
#        define KEYVALUE_API __declspec(dllimport)
#   endif
#else
#   define KEYVALUE_API
#endif

typedef void* keyvalue_handle_t;

/* Open a keyvalue file and return a handle to it 
 */
KEYVALUE_API keyvalue_handle_t keyvalue_open(const char *fn, const char* sections[] );

/* Get all the keys parsed
 */
KEYVALUE_API const char**  keyvalue_getkeys(keyvalue_handle_t kv_handle );

/* Get a value given a key
 */
KEYVALUE_API const char* keyvalue_getstring(keyvalue_handle_t kv_handle, const char *key);

/* Close a keyvalue file handle 
 */
KEYVALUE_API void keyvalue_close(keyvalue_handle_t kv_handle);

#ifdef __cplusplus
}
#endif

#endif
