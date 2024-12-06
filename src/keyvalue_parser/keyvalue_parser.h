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

#ifndef __KEYVALUE_PARSER_H
#define __KEYVALUE_PARSER_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef WIN32
#ifdef LIBKEYVALUE_PARSER_EXPORTS
#define KEYVALUE_API __declspec(dllexport)
#else
#define KEYVALUE_API __declspec(dllimport)
#endif
#else
#define KEYVALUE_API
#endif

    typedef void* keyvalue_handle_t;
    typedef void keyvalue_type_t;

    /* Open a keyvalue file and return a handle to it
 */
    KEYVALUE_API keyvalue_handle_t keyvalue_open(const char* fn, const char* const sections[]);

    /* Get all the keys parsed
 */
    KEYVALUE_API const char** keyvalue_getkeys(keyvalue_handle_t kv_handle);

    /* Get a value given a key
 */
    KEYVALUE_API const char* keyvalue_getstring(keyvalue_handle_t kv_handle, const char* key);

    /* Close a keyvalue file handle
 */
    KEYVALUE_API void keyvalue_close(keyvalue_handle_t kv_handle);

#ifdef __cplusplus
}
#endif

#endif
