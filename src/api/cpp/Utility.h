

namespace gravity {

std::string StringToLowerCase(std::string str);
char* StringToLowerCase(char* str, int leng);
std::string StringCopyToLowerCase(const std::string &str);

int IntToString(std::string str, int default_value);

bool IsValidFilename(const std::string filename);

/**
 * Get the time in microseconds from the Unix epoch.
 */
uint64_t getCurrentTime();

}
