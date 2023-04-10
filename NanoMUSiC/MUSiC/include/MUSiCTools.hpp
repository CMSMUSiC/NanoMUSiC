#ifndef MUSIC_TOOLS
#define MUSIC_TOOLS

#include <algorithm>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <map>
#include <optional>
#include <random>
#include <regex>
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

namespace MUSiCTools
{

typedef boost::filesystem::path Path;

class value_error : public std::runtime_error
{
  public:
    value_error(std::string const &msg)
        : std::runtime_error(msg)
    {
    }
};

class config_error : public std::runtime_error
{
  public:
    config_error(std::string const &msg)
        : std::runtime_error(msg)
    {
    }
};

class unsorted_error : public std::runtime_error
{
  public:
    unsorted_error(std::string const &msg)
        : std::runtime_error(msg)
    {
    }
};

class file_not_found : public std::exception
{
  public:
    file_not_found(std::string const &filename, std::string const &filetype = "")
        : m_filename(filename),
          m_filetype(filetype)
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
inline std::string musicAbsPath(std::string relPath)
{
    if (relPath.substr(0, 1) == "/")
        return relPath;
    std::string output;
    char *pPath = std::getenv("PXLANALYZER_BASE");
    if (pPath != NULL)
    {
        output = std::string(pPath) + "/" + relPath;
    }
    else
    {
        std::cout << "FATAL: PXLANALYZER_BASE not set!" << std::endl;
        output = "";
    }
    return output;
}

// Remove comment from line.
inline std::string removeComment(std::string line, char const commentChar = '#')
{
    if (line.empty())
    {
        return line;
    }

    std::string::size_type pos = line.find_first_of(commentChar);
    if (pos != std::string::npos)
    {
        line.erase(pos);
    }

    boost::trim(line);

    return line;
}

inline std::string random_string(size_t length)
{
    auto randchar = []() -> char {
        const char charset[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[rand() % max_index];
    };
    std::string str(length, 0);
    std::generate_n(str.begin(), length, randchar);
    return str;
}

// return a vector of string identifiers for each physics object type
inline std::vector<std::string> getParticleTypeAbbreviations(bool isRec = true)
{
    std::vector<std::string> partList;
    partList.push_back("Ele");
    partList.push_back("Muon");
    partList.push_back("Tau");
    partList.push_back("Gamma");
    partList.push_back("FatJet");
    partList.push_back("Jet");
    partList.push_back("MET");

    if (!isRec)
    {
        // 3rd generation
        partList.push_back("Top");
        partList.push_back("b");
        // neutrinos
        partList.push_back("Nu_ele");
        partList.push_back("Nu_muon");
        partList.push_back("Nu_tau");
        // bosons
        partList.push_back("Gluon");
        partList.push_back("Z");
        partList.push_back("W");
        partList.push_back("H");
    }
    return partList;
}

inline std::map<int, std::string> pdg_id_type_map(bool useBJet = false)
{
    std::map<int, std::string> outMap = std::map<int, std::string>();
    outMap.emplace(11, "Ele");
    outMap.emplace(12, "Nu_ele");
    outMap.emplace(13, "Muon");
    outMap.emplace(14, "Nu_muon");
    outMap.emplace(15, "Tau");
    outMap.emplace(16, "Nu_tau");

    // treat all quarks as jets
    outMap.emplace(1, "Jet");
    outMap.emplace(2, "Jet");
    outMap.emplace(3, "Jet");
    outMap.emplace(4, "Jet");
    outMap.emplace(5, "b");
    outMap.emplace(6, "Top");
    outMap.emplace(7, "Jet");
    outMap.emplace(8, "Jet");
    // Bosons
    outMap.emplace(21, "Gluon");
    outMap.emplace(9, "Gluon");
    outMap.emplace(22, "Gamma");
    outMap.emplace(23, "Z");
    outMap.emplace(24, "W");
    outMap.emplace(25, "H");
    return outMap;
}

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
void splitString(std::vector<T> &result,
                 const std::string &input,
                 const std::string &sep = ",",
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

inline std::string parse_and_expand_music_base(std::string_view path)
{
    return std::regex_replace(std::string(path), std::regex("\\$MUSIC_BASE"), std::string(std::getenv("MUSIC_BASE")));
}

template <typename T>
size_t ArgMin(const std::vector<T> &seq)
{
    return std::distance(seq.begin(), std::min_element(seq.begin(), seq.end()));
}

template <typename T>
std::optional<T> MinElem(const std::vector<T> &seq)
{
    if (seq.size() > 0)
    {
        return seq.at(ArgMin(seq));
    }
    return std::nullopt;
}

inline double generate_uniform()
{
    static std::default_random_engine e;
    static std::uniform_real_distribution<> dis(0, std::nextafter(1, std::numeric_limits<double>::max())); // rage 0 - 1
    return dis(e);
}

} // namespace MUSiCTools
#endif /*MUSIC_TOOLS*/
