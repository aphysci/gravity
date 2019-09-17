/* (C) Copyright 2013, Applied Physical Sciences Corp., A General Dynamics Company
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

#ifndef GRAVITY_UTILITY_H__
#define GRAVITY_UTILITY_H__
#include <string>
#include <stdint.h>

/**
 * Mark functions that are part of the public API.
 */
#ifdef _WIN32
#    if _WIN32 != __MINGW32__
#        ifdef GRAVITY_EXPORTS
#            define GRAVITY_API __declspec(dllexport)
#        else
#            define GRAVITY_API __declspec(dllimport)
#        endif
#    else
#        define GRAVITY_API
#    endif
#else
#    define GRAVITY_API
#endif

namespace gravity {

/**
 * @name String conversion and helper functions
 * @{
 */ 
GRAVITY_API std::string StringToLowerCase(std::string str); ///< Return lowercase string
GRAVITY_API char* StringToLowerCase(char* str, int leng); ///< In-place modification and returns ptr to string
GRAVITY_API std::string StringCopyToLowerCase(const std::string &str); ///< \TODO remove, same as StringToLowerCase
GRAVITY_API int StringToInt(std::string str, int default_value); ///< Return an integer.
GRAVITY_API double StringToDouble(std::string str, double default_value); ///< Return a double
/**
 * Trim a string.
 * \param s string that will be modified
 * \param delimiters characters to remove from the right and left of the string
 * \return also return a reference to the modified string
 */
GRAVITY_API std::string& trim(std::string& s, const std::string& delimiters = " \f\n\r\t\v" );

/**
 * Replace substrings in a string.
 * \param target string that will be modified
 * \param oldValue source substring to search for in target
 * \param newValue replacement substring
 */
GRAVITY_API void replaceAll(std::string& target, const std::string& oldValue, const std::string& newValue);
/** @} */ //String conversion and helper functions

GRAVITY_API bool IsValidFilename(const std::string filename); ///< Return if filename is valid, which is OS dependent

/**
 * @name Time functions
 */
/**
 * Get the current system time in Unix epoch microseconds.
 * Only works on Unix systems.
 * \TODO place an ifdef around this
 */
GRAVITY_API uint64_t getCurrentTime();
/** @} */ //Time functions

GRAVITY_API unsigned int sleep(int milliseconds);

}

#endif //GRAVITY_UTILITY_H__
