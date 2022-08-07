#ifndef STRING_UTILITIES_H
#define STRING_UTILITIES_H

#include <string>
#include <vector>

#define WHITESPACE " \r\n\t\v\f"

namespace string_utilities {

void trim(std::string& str);

std::vector<std::string> split(std::string& str, char delimeter = ':');

std::vector<std::string> split_whitespace(std::string& str);

std::string messageFromHttpResponseCode(unsigned int httpResponseCode);

}

#endif // STRING_UTILITIES_H
