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

#ifndef GRAVITY_UTILITY_H__
#define GRAVITY_UTILITY_H__
#include <cstdint>
#include <string>

#ifdef _WIN32
#if _WIN32 != __MINGW32__
#    ifdef GRAVITY_EXPORTS
#        define GRAVITY_API __declspec(dllexport)
#    else
#        define GRAVITY_API __declspec(dllimport)
#    endif
#else
#    define GRAVITY_API
#endif
#else
#define GRAVITY_API
#endif

namespace gravity {

GRAVITY_API std::string StringToLowerCase(std::string str);
GRAVITY_API char* StringToLowerCase(char* str, int leng);
GRAVITY_API std::string StringCopyToLowerCase(const std::string &str);

GRAVITY_API int StringToInt(std::string str, int default_value);
GRAVITY_API double StringToDouble(std::string str, int default_value);

GRAVITY_API bool IsValidFilename(const std::string filename);

GRAVITY_API uint64_t getCurrentTime();  ///< Utility method to get the current system time in epoch microseconds

GRAVITY_API unsigned int sleep(int milliseconds);
GRAVITY_API std::string& trim(std::string& s, const std::string& delimiters = " \f\n\r\t\v" );

GRAVITY_API void replaceAll(std::string& target, const std::string& oldValue, const std::string& newValue);

}

#endif //GRAVITY_UTILITY_H__
