#ifndef GRAVITY_UTILITY_H__
#define GRAVITY_UTILITY_H__
#include <string>

namespace gravity {

std::string StringToLowerCase(std::string str);
char* StringToLowerCase(char* str, int leng);
std::string StringCopyToLowerCase(const std::string &str);

int StringToInt(std::string str, int default_value);
double StringToDouble(std::string str, int default_value);

bool IsValidFilename(const std::string filename);

/**
 * Get the time in microseconds from the Unix epoch.
 */
uint64_t getCurrentTime();

unsigned int sleep(int milliseconds);
std::string& trim(std::string& s, const std::string& delimiters = " \f\n\r\t\v" );


}

#endif //GRAVITY_UTILITY_H__
