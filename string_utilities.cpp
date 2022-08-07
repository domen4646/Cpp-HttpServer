#include "string_utilities.h"
#include <map>


void string_utilities::trim(std::string& str)
{
    str.erase(str.find_last_not_of(WHITESPACE) + 1);
    str.erase(0, str.find_first_not_of(WHITESPACE));
}

std::vector<std::string> string_utilities::split(std::string& str, char delimeter)
{
    std::vector<std::string> result;
    std::string current = "";
    for (char c : str) {
        if (c == delimeter) {
            if (current.empty()) {
                continue;
            }
            result.push_back(current);
            current.clear();
            continue;
        }
        current.push_back(c);
    }

    if (!current.empty()) {
        result.push_back(current);
    }
    return result;
}

std::vector<std::string> string_utilities::split_whitespace(std::string& str)
{
    std::vector<std::string> result;
    std::string current = "";
    for (char c : str) {
        if (isspace(c)) {
            if (current.empty()) {
                continue;
            }
            result.push_back(current);
            current.clear();
            continue;
        }
        current.push_back(c);
    }

    if (!current.empty()) {
        result.push_back(current);
    }
    return result;
}

std::map<unsigned int, std::string> httpCodes = {
    {200, "OK"},
    {404, "Not Found"}
};

std::string string_utilities::messageFromHttpResponseCode(unsigned int httpResponseCode)
{
    if (httpCodes.count(httpResponseCode) > 0) {
        return httpCodes[httpResponseCode];
    }
    return "Unknown";
}
