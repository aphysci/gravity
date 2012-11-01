#ifndef GRAVITY_UTILITY_H__
#define GRAVITY_UTILITY_H__
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

#include <stdint.h>

namespace gravity {

GRAVITY_API std::string StringToLowerCase(std::string str);
GRAVITY_API char* StringToLowerCase(char* str, int leng);
GRAVITY_API std::string StringCopyToLowerCase(const std::string &str);

GRAVITY_API int StringToInt(std::string str, int default_value);
GRAVITY_API double StringToDouble(std::string str, int default_value);

GRAVITY_API bool IsValidFilename(const std::string filename);

/**
 * Get the time in microseconds from the Unix epoch.
 */
GRAVITY_API uint64_t getCurrentTime();

GRAVITY_API unsigned int sleep(int milliseconds);
GRAVITY_API std::string& trim(std::string& s, const std::string& delimiters = " \f\n\r\t\v" );


}

#endif //GRAVITY_UTILITY_H__
