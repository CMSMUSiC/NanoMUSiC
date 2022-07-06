#include "util.h"

std::string str_replace(std::string s,
        const std::string needle,
        const std::string replacement){
    return s.replace(s.find(needle), needle.length(), replacement);
}

bool ends_with(std::string const & value, std::string const & ending){
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

bool starts_with(std::string const & value, std::string const & start){
    if (start.size() > value.size()) return false;
    return std::equal(start.begin(), start.end(), value.begin());
}

