#ifndef UTIL_H
#define UTIL_H

#include <string>

std::string str_replace(std::string s, const std::string needle, const std::string replacement);
bool ends_with(std::string const & value, std::string const & ending);
bool starts_with(std::string const & value, std::string const & start);

#endif // UTIL_H
