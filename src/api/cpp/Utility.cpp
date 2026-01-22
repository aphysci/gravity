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

#include <string>
#include <sstream>

#include "Utility.h"

#ifdef WIN32
#define NOMINMAX
#include <Windows.h>
#include <time.h>
#include <algorithm>
#if _MSC_VER < 1910 && !defined(_CRT_NO_TIME_T)
struct timespec
{
    time_t tv_sec;   // Seconds - >= 0
    time_t tv_nsec;  // Nanoseconds - [0, 999999999]
};
#endif
#else
#include <stdint.h>
#endif
#include <thread>
#include <chrono>
namespace gravity
{

// String Conversions
std::string StringToLowerCase(std::string str)
{
    std::use_facet<std::ctype<char> >(std::locale(""))
        .tolower(&str[0], &str[0] + str.length());  //Convert to lowercase.
    return str;
}
char* StringToLowerCase(char* str, int leng)
{
    std::use_facet<std::ctype<char> >(std::locale("")).tolower(&str[0], &str[0] + leng);  //Convert to lowercase.

    return str;
}

std::string StringCopyToLowerCase(const std::string& str)
{
    std::string copy = str;
    return StringToLowerCase(copy);
}

int StringToInt(std::string str, int default_value)
{
    int ret_val;
    std::stringstream ss(str);
    ss >> ret_val;
    if (ss.fail()) ret_val = default_value;
    return ret_val;
}

double StringToDouble(std::string str, double default_value)
{
    double ret_val;
    std::stringstream ss(str);
    ss >> ret_val;
    if (ss.fail()) ret_val = default_value;
    return ret_val;
}

//Trimming
std::string& trim_right_inplace(
    std::string& s, const std::string& delimiters = " \f\n\r\t\v")
{
    return s.erase(s.find_last_not_of(delimiters) + 1);
}

std::string& trim_left_inplace(
    std::string& s, const std::string& delimiters = " \f\n\r\t\v")
{
    return s.erase(0, s.find_first_not_of(delimiters));
}

std::string& trim(std::string& s, const std::string& delimiters)
{
    return trim_left_inplace(trim_right_inplace(s, delimiters), delimiters);
}

void replaceAll(std::string& target, const std::string& oldValue, const std::string& newValue)
{
    //necessary check - otherwise program crashes
    if (oldValue.empty()) return;
    auto pos = target.find(oldValue);
    while (pos != std::string::npos)
    {
        target.replace(pos, oldValue.size(), newValue);
        pos = target.find(oldValue);
    }
}

// OS
bool IsValidFilename(const std::string filename)
{
    char restrictedChars[] = "/\\?%*:|\"<>";
    const size_t numRChars = 10;
    size_t numDots = 0;

    for (size_t i = 0; i < filename.length(); i++)
    {
        if (filename[i] < 31) return false;

        for (size_t r = 0; r < numRChars; r++)
            if (filename[i] == restrictedChars[r]) return false;

        if (filename[i] == '.') numDots++;
    }

    //TODO: on windows check for specific restrictions (cannot be complete first segment before .):
    //CON, PRN, AUX, CLOCK$, NUL, COM0, COM1, COM2, COM3, COM4, COM5, COM6, COM7, COM8, COM9, LPT0, LPT1, LPT2, LPT3, LPT4, LPT5, LPT6, LPT7, LPT8, and LPT9.
    //And not the following for NTFS: $Mft, $MftMirr, $LogFile, $Volume, $AttrDef, $Bitmap, $Boot, $BadClus, $Secure, $Upcase, $Extend, $Quota, $ObjId and $Reparse

    if (numDots == filename.length()) return false;  //We can't be all dots.

    return true;
}


uint64_t getCurrentTime()
{
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
    return (uint64_t) microseconds;
    // timespec ts;
    // clock_gettime(0, &ts);
    // return (uint64_t)ts.tv_sec * 1000000LL + (uint64_t)ts.tv_nsec / 1000LL;  //in microseconds
}

unsigned int sleep(int milliseconds)
{
    // If sleep time < 0, set it to 0
    milliseconds = std::max(0, milliseconds);
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));  //Maybe replace this guy with clock_nanosleep???
    return 0;
}

}  // namespace gravity
