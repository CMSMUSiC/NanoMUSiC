#ifndef MUSIC_TOOLS
#define MUSIC_TOOLS

#include <cstdlib>
#include <exception>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <vector>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#pragma GCC diagnostic pop

#include "TSystem.h" // for ExpandPathName

namespace Tools
{
typedef boost::filesystem::path Path;

class value_error : public std::runtime_error
{
  public:
    value_error(std::string const &msg) : std::runtime_error(msg)
    {
    }
};

class config_error : public std::runtime_error
{
  public:
    config_error(std::string const &msg) : std::runtime_error(msg)
    {
    }
};

class unsorted_error : public std::runtime_error
{
  public:
    unsorted_error(std::string const &msg) : std::runtime_error(msg)
    {
    }
};

class file_not_found : public std::exception
{
  public:
    file_not_found(std::string const &filename, std::string const &filetype = "")
        : m_filename(filename), m_filetype(filetype)
    {
    }
    ~file_not_found() throw()
    {
    }
    virtual const char *what() const throw()
    {
        std::stringstream error;
        if (m_filetype.empty())
            error << "File '" << m_filename << "' not found!";
        else
            error << m_filetype << " '" << m_filename << "' not found!";
        return error.str().c_str();
    }

  private:
    std::string m_filename;
    std::string m_filetype;
};

// returns the abolute path to file given with a path relative to PXLANALYZER_BASE
// returns the given path if it is already absolute (starts with a /)
std::string musicAbsPath(std::string relPath);

// Remove comment from line.
std::string removeComment(std::string line, char const commentChar = '#');
std::string random_string(size_t length);
// returb a vector of string identifiers for each physics object type
std::vector<std::string> getParticleTypeAbbreviations(bool isRec = true);
std::map<int, std::string> pdg_id_type_map(bool useBJet = false);
// return everything you can << into an ostream as a string
template <class T>
std::string toString(T &input)
{
    std::stringstream out;
    out << input;
    return out.str();
}

// convert a string into anything you can >> from an istream (this is what boost::lexical_cast basically does)
template <class T>
T inline fromString(const std::string &valuestring)
{
    T value = T();

    if (!valuestring.empty())
        value = boost::lexical_cast<T>(valuestring);

    return value;
}

// if the return type is 'string', than you don't have to convert it to 'string' any more
template <>
std::string inline fromString<std::string>(const std::string &input)
{
    return input;
}

// splits the input string at each occurence of sep and puts the parts into the result vector
// if ignore empty is not set, the output vector will contain default values for repeated separators
template <class T>
void splitString(std::vector<T> &result, const std::string &input, const std::string &sep = ",",
                 bool ignoreEmpty = false)
{
    result.clear();
    if (ignoreEmpty && input.empty())
        return;

    std::string::size_type pos = input.find(sep);
    std::string::size_type pos_old = 0;

    size_t sep_len = sep.size();

    while (std::string::npos != pos)
    {
        size_t len = pos - pos_old;

        if (len != 0)
        {
            std::string substring = input.substr(pos_old, len);
            substring = boost::trim_copy(substring);
            T temp = T();
            if (!substring.empty())
                temp = fromString<T>(substring);
            result.push_back(temp);
        }
        else if (!ignoreEmpty)
        {
            result.push_back(T());
        }

        pos_old = pos + sep_len;

        pos = input.find(sep, pos_old);
    }

    if (pos_old != input.size())
    {
        std::string substring = input.substr(pos_old);
        substring = boost::trim_copy(substring);
        T temp = T();
        if (!substring.empty())
            temp = fromString<T>(substring);
        result.push_back(temp);
    }
    else if (!ignoreEmpty)
    {
        result.push_back(T());
    }
}

template <class T>
std::vector<T> splitString(const std::string &input, const bool ignoreEmpty = false, const std::string sep = ",")
{
    std::vector<T> result;
    splitString(result, input, sep, ignoreEmpty);
    return result;
}

// This is just a wrapper for the ROOT function.
// If you pass a 'std::string' it's implicitly converted to a
// 'boost::filesystem::path', so both are accepted as input here.
inline std::string ExpandPath(Path const &path)
{
    return std::string(gSystem->ExpandPathName(path.string().c_str()));
}

inline std::string AbsolutePath(Path const &path)
{
    Path const AbsPath(ExpandPath(path));
    return complete(AbsPath).string();
}
} // namespace Tools

#endif /*MUSIC_TOOLS*/
