#include <string>
#include <sstream>
#include <pthread.h>

#include "Utility.h"

#ifdef WIN32
#include <Windows.h>
#else
#include <stdint.h>
#include <sys/unistd.h>
#endif

namespace gravity {

////////////////////////////
// String Helpers

//In Place case conversion.
GRAVITY_API std::string StringToLowerCase(std::string str)
{
	std::use_facet< std::ctype<char> >(std::locale("")).tolower(&str[0], &str[0] + str.length()); //Convert to lowercase.
	return str;
}
GRAVITY_API char* StringToLowerCase(char* str, int leng)
{
	std::use_facet< std::ctype<char> >(std::locale("")).tolower(&str[0], &str[0] + leng); //Convert to lowercase.

	return str;
}

//Copying case conversion
GRAVITY_API std::string StringCopyToLowerCase(const std::string &str)
{
	std::string copy = str;
	return StringToLowerCase(copy);
}

//Conversions
GRAVITY_API int StringToInt(std::string str, int default_value)
{
	int ret_val;
	std::stringstream ss(str);
	ss >> ret_val;
	if(ss.fail())
		ret_val = default_value;
	return ret_val;
}

GRAVITY_API double StringToDouble(std::string str, double default_value)
{
	double ret_val;
	std::stringstream ss(str);
	ss >> ret_val;
	if(ss.fail())
		ret_val = default_value;
	return ret_val;
}

//Trimming
GRAVITY_API std::string& trim_right_inplace(
  std::string&       s,
  const std::string& delimiters = " \f\n\r\t\v" )
{
  return s.erase( s.find_last_not_of( delimiters ) + 1 );
}

GRAVITY_API std::string& trim_left_inplace(
  std::string&       s,
  const std::string& delimiters = " \f\n\r\t\v" )
{
  return s.erase( 0, s.find_first_not_of( delimiters ) );
}

GRAVITY_API std::string& trim(
  std::string&       s,
  const std::string& delimiters)
{
  return trim_left_inplace( trim_right_inplace( s, delimiters ), delimiters );
}

// OS
GRAVITY_API bool IsValidFilename(const std::string filename)
{
	char restrictedChars[] = "/\\?%*:|\"<>";
	const size_t numRChars = 10;
	size_t numDots = 0;

	for(size_t i = 0; i < filename.length(); i++)
	{
		if(filename[i] < 31)
			return false;

		for(size_t r = 0; r < numRChars; r++)
			if(filename[i] == restrictedChars[r])
				return false;

		if(filename[i] == '.')
			numDots++;
	}

	//TODO: on windows check for specific restrictions (cannot be complete first segment before .):
	//CON, PRN, AUX, CLOCK$, NUL, COM0, COM1, COM2, COM3, COM4, COM5, COM6, COM7, COM8, COM9, LPT0, LPT1, LPT2, LPT3, LPT4, LPT5, LPT6, LPT7, LPT8, and LPT9.
	//And not the following for NTFS: $Mft, $MftMirr, $LogFile, $Volume, $AttrDef, $Bitmap, $Boot, $BadClus, $Secure, $Upcase, $Extend, $Quota, $ObjId and $Reparse

	if(numDots == filename.length())
		return false;  //We can't be all dots.

	return true;
}


//Replace the clock_gettime for Windows.
#ifdef WIN32
//From http://stackoverflow.com/questions/5404277/porting-clock-gettime-to-windows
#include <Windows.h>
LARGE_INTEGER
getFILETIMEoffset()
{
    SYSTEMTIME s;
    FILETIME f;
    LARGE_INTEGER t;

    s.wYear = 1970;
    s.wMonth = 1;
    s.wDay = 1;
    s.wHour = 0;
    s.wMinute = 0;
    s.wSecond = 0;
    s.wMilliseconds = 0;
    SystemTimeToFileTime(&s, &f);
    t.QuadPart = f.dwHighDateTime;
    t.QuadPart <<= 32;
    t.QuadPart |= f.dwLowDateTime;
    return (t);
}

//T. Ludwinski: changed timeval to timespec and microseconds to nanoseconds
int
clock_gettime(int X, struct timespec *tv)
{
    LARGE_INTEGER           t;
    FILETIME            f;
    double                  nanoseconds;
    static LARGE_INTEGER    offset;
    static double           frequencyToNanoseconds;
    static int              initialized = 0;
    static BOOL             usePerformanceCounter = 0;
    static LARGE_INTEGER 	startTime;

    if (!initialized) {
        LARGE_INTEGER performanceFrequency;
        initialized = 1;
        usePerformanceCounter = QueryPerformanceFrequency(&performanceFrequency);
        if (usePerformanceCounter) {
            QueryPerformanceCounter(&offset);
            frequencyToNanoseconds = (double)performanceFrequency.QuadPart / 1000000000.;
        } else {
            offset = getFILETIMEoffset();
            frequencyToNanoseconds = .01;
        }

        GetSystemTimeAsFileTime(&f);
        startTime.QuadPart = f.dwHighDateTime;
        startTime.QuadPart <<= 32;
        startTime.QuadPart |= f.dwLowDateTime;

        startTime.QuadPart -= 116444736000000000ULL; //Convert from Window time to UTC (100ns)
        startTime.QuadPart = startTime.QuadPart * 100; //To nanoseconds
    }
    if (usePerformanceCounter) QueryPerformanceCounter(&t);
    else {
        GetSystemTimeAsFileTime(&f);
        t.QuadPart = f.dwHighDateTime;
        t.QuadPart <<= 32;
        t.QuadPart |= f.dwLowDateTime;
    }

    t.QuadPart -= offset.QuadPart;
    nanoseconds = (double)t.QuadPart / frequencyToNanoseconds;
    nanoseconds += startTime.QuadPart;
    t.QuadPart = (LONGLONG) nanoseconds;
    tv->tv_sec = t.QuadPart / 1000000000LL;
    tv->tv_nsec = t.QuadPart % 1000000000LL;
    return (0);
}
#endif

//In Microseconds
GRAVITY_API uint64_t getCurrentTime()
{
    timespec ts;
    clock_gettime(0, &ts);
    return (uint64_t)ts.tv_sec * 1000000LL + (uint64_t)ts.tv_nsec / 1000LL;
}

GRAVITY_API unsigned int sleep(int milliseconds)
{
#ifdef WIN32
	Sleep(milliseconds);
	return 0;
#else
	return usleep(milliseconds*1000); //Maybe replace this guy with clock_nanosleep???
#endif
}

}
