#ifndef TOOLS_MCONFIG
#define TOOLS_MCONFIG

#if ((__GNUC__ * 100) + __GNUC_MINOR__) >= 402
#define GCC_DIAG_STR(s) #s
#define GCC_DIAG_JOINSTR(x, y) GCC_DIAG_STR(x##y)
#define GCC_DIAG_DO_PRAGMA(x) _Pragma(#x)
#define GCC_DIAG_PRAGMA(x) GCC_DIAG_DO_PRAGMA(GCC diagnostic x)
#if ((__GNUC__ * 100) + __GNUC_MINOR__) >= 406
#define GCC_DIAG_OFF(x)                                                                                                \
    GCC_DIAG_PRAGMA(push)                                                                                              \
    GCC_DIAG_PRAGMA(ignored GCC_DIAG_JOINSTR(-W, x))
#define GCC_DIAG_ON(x) GCC_DIAG_PRAGMA(pop)
#else
#define GCC_DIAG_OFF(x) GCC_DIAG_PRAGMA(ignored GCC_DIAG_JOINSTR(-W, x))
#define GCC_DIAG_ON(x) GCC_DIAG_PRAGMA(warning GCC_DIAG_JOINSTR(-W, x))
#endif
#else
#define GCC_DIAG_OFF(x)
#define GCC_DIAG_ON(x)
#endif

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>

// GCC_DIAG_OFF(unused - local - typedefs);
#include <boost/algorithm/string.hpp>
// GCC_DIAG_ON(unused - local - typedefs);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#pragma GCC diagnostic pop

#include "Tools.hh"

namespace Tools
{

class MConfig
{
    typedef boost::filesystem::path Path;

  public:
    // constructor for MConfig objects
    explicit MConfig(const std::string &path = "", const std::string &configBasePath = "", const char commentChar = '#',
                     const char delimiterChar = '=', const char quoteChar = '"', const bool warnOverwrite = false)
        : m_configInit(false), m_writeFile(false), m_warnOverwrite(false), m_commentChar(commentChar),
          m_delimiterChar(delimiterChar), m_quoteChar(quoteChar), m_configBasePath(Tools::ExpandPath(configBasePath)),
          m_configFilePath(Tools::ExpandPath(path)), m_configFileName(m_configBasePath.filename()), m_writePath("."),
          m_writeFileName(m_configFilePath.filename()), m_importIdentifier("import"),
          m_modifiedImportIdentifier1("with"), m_modifiedImportIdentifier2("as")
    {
        // the different steering character must not be the same!
        if (m_commentChar == m_delimiterChar)
            throw std::runtime_error(
                "MConfig::MConfig(...): It is not allowed that m_commentChar == m_delimiterChar !");
        if (m_commentChar == m_quoteChar)
            throw std::runtime_error("MConfig::MConfig(...): It is not allowed that m_commentChar == m_quoteChar !");
        if (m_delimiterChar == m_quoteChar)
            throw std::runtime_error("MConfig::MConfig(...): It is not allowed that m_delimiterChar == m_quoteChar !");

        // try to read data from disk only if a path has been specified
        if (!m_configFilePath.empty())
        {
            ImportFile(m_configFilePath.string());
        };
    }

    // destructor tries to write config data into file, only if the write flag has been set to true
    ~MConfig()
    {
        WriteFile();
    }

    void AddItem(const std::string &key, const std::string &value)
    {
        std::map<std::string, std::string>::iterator pos = m_configMap.find(key);

        if (pos != m_configMap.end() and m_warnOverwrite)
        {
            std::cerr << "WARNING: Overwriting value of key: " << key << std::endl;
            std::cerr << "         From configuration file: " << m_configFilePath << std::endl;
        }

        m_configMap[key] = value;
    }

    bool CheckItem(const std::string &key);

    void ClearItems()
    {
        m_configMap.clear();
    }

    template <class T> T GetItem(const std::string &key, const T &defaultItem) const
    {
        std::map<std::string, std::string>::const_iterator it = m_configMap.find(key);

        if (it == m_configMap.end())
            return defaultItem;

        return Tools::fromString<T>((*it).second);
    }

    template <class T> T GetItem(const std::string &key) const
    {
        std::map<std::string, std::string>::const_iterator it = m_configMap.find(key);

        if (it == m_configMap.end())
            throw std::runtime_error("MConfig::GetItem(...): key '" + key + "' not found and no default item provided");

        return Tools::fromString<T>((*it).second);
    }

    // take a path and try to open and read am ASCII file with configuration information
    void ImportFile(const std::string &iPath, const std::string &old_string = "", const std::string &new_string = "")
    {
        Path importPath = boost::trim_copy(Tools::ExpandPath(iPath));
        Path absPath = Init(importPath);
        std::ifstream configFile(absPath.string().c_str());

        if (!configFile.is_open())
            throw std::runtime_error("MConfig::ImportFile(): Failed to open Config File: '" + absPath.string() + "'");

        unsigned int lineNum = 0;
        while (configFile.good() && !configFile.eof())
        {
            std::string line;
            getline(configFile, line);
            line = Tools::removeComment(line, m_commentChar);
            ++lineNum;

            ScanForImports(line, lineNum, absPath);

            if (!line.empty())
            {
                // search for the first appearance of delimiterChar and quoteChar
                std::string::size_type foundDelim = line.find_first_of(m_delimiterChar);
                std::string::size_type foundQuote = line.find_first_of(m_quoteChar);

                // if there is no delimiter found in the line, something is wrong
                if (foundDelim == std::string::npos)
                {
                    std::stringstream error;
                    error << "MConfig::ImportFile( const std::string &path ): No delimiter '" << m_delimiterChar
                          << "' found on line number " << lineNum << " in file '" << absPath << "'. Please check it!";
                    throw std::runtime_error(error.str());

                    // it is allowed to have a delimiterChar inside the quotation, e.g.: a = "b = c"
                    // but there must be a delimiter before the first quoteChar
                }
                else if (foundQuote < foundDelim)
                {
                    std::stringstream error;
                    error << "MConfig::ImportFile( const std::string &path ): Quotation character '" << m_quoteChar
                          << "' found before delimiter '" << m_delimiterChar << "' on line number " << lineNum
                          << " in file '" << absPath << "'. Please check it!";
                    throw std::runtime_error(error.str());
                }

                std::string key(line.begin(), line.begin() + foundDelim);
                std::string value = line.substr(foundDelim + 1);

                // now search for delimiterChar and quoteChar in the value string
                foundDelim = value.find_first_of(m_delimiterChar);
                foundQuote = value.find_first_of(m_quoteChar);

                // if the delimiter is not inside the quotation now, something is wrong
                if (foundDelim < foundQuote)
                {
                    std::stringstream error;
                    error << "MConfig::ImportFile( const std::string &path ): More than one delmiter '"
                          << m_delimiterChar << "' found on line number " << lineNum << " in file '" << absPath
                          << "'. Please check it!";
                    throw std::runtime_error(error.str());
                }

                boost::trim(key);
                boost::trim(value);
                value = ExtractFromQuotes(value, lineNum, absPath);

                if (old_string != "" and key.find(old_string) != std::string::npos)
                {
                    size_t pos = key.find(old_string);
                    key.replace(pos, old_string.length(), new_string);
                }
                AddItem(key, value);
            }
        }

        configFile.close();
    }

    // print the content of the config map
    void Print();

    // dump to file the config map
    void DumpToFile(std::string filePath);

    // remove year specific configurations
    void setYear(std::string year, std::vector<std::string> possibleYears = {"2016APV", "2016", "2017", "2018"});

    // remove item with name itemtag from configuration map
    // returns true if the item has been found and removed successful
    bool RemoveItem(const std::string &itemtag);

    void WriteFile(bool forceWrite = false)
    {
        if (!m_writeFile && !forceWrite)
            return;
        if (forceWrite)
            SetWritePathAndName();

        Path pathandname = m_writePath / m_writeFileName;

        std::ofstream ofs(pathandname.string().c_str());
        if (!ofs.is_open())
            throw std::runtime_error("MConfig::WriteFile(): Could not write to file: '" + pathandname.string() + "'");

        int width = FindLongestKey();
        for (std::map<std::string, std::string>::iterator it = m_configMap.begin(); it != m_configMap.end(); ++it)
        {
            ofs << std::setw(width) << setiosflags(std::ios::left) << (*it).first << " " << m_delimiterChar << " "
                << (*it).second << std::endl;
        }

        ofs.close();
    }

    void SetWritePathAndName(const std::string &writePath = "", const std::string &writeFileName = "")
    {
        if (writePath.empty())
            m_writePath = (std::string)gSystem->pwd();
        else
            m_writePath = Tools::ExpandPath(writePath);

        if (writeFileName.empty())
            m_writeFileName = m_configFileName;
        else
            m_writeFileName = writeFileName;
    }

    void SetWriteFile(const bool &write = true, const std::string &writePath = "",
                      const std::string &writeFileName = "")
    {
        SetWritePathAndName(writePath, writeFileName);

        m_writeFile = write;
    }

    const char GetCommentChar() const
    {
        return m_commentChar;
    }
    const char GetDelimiterChar() const
    {
        return m_delimiterChar;
    }
    const char GetQuoteChar() const
    {
        return m_quoteChar;
    }
    std::string GetConfigBasePath() const
    {
        return m_configBasePath.string();
    }
    std::string GetConfigFilePath() const
    {
        return m_configFilePath.string();
    }
    std::string GetWritePath() const
    {
        return m_writePath.string();
    }
    std::string GetWriteFileName() const
    {
        return m_writeFileName.string();
    }
    std::string GetImportIdentifier() const
    {
        return m_importIdentifier;
    }

    void SetCommentChar(const char commentChar)
    {
        m_commentChar = commentChar;
    }
    void SetDelimiterChar(const char delimiterChar)
    {
        m_delimiterChar = delimiterChar;
    }
    void SetQuoteChar(const char quoteChar)
    {
        m_quoteChar = quoteChar;
    }
    void SetConfigBasePath(const std::string &path)
    {
        m_configBasePath = Tools::ExpandPath(path);
    }
    void SetConfigBasePath(const Path &path)
    {
        m_configBasePath = path;
    }
    void SetImportIdentifier(const std::string &identfier)
    {
        m_importIdentifier = identfier;
    }

  private:
    bool m_configInit;
    bool m_writeFile;
    bool m_warnOverwrite;
    char m_commentChar;
    char m_delimiterChar;
    char m_quoteChar;

    Path m_configBasePath;
    Path m_configFilePath;
    Path m_configFileName;
    Path m_writePath;
    Path m_writeFileName;

    std::string m_importIdentifier;
    std::string m_modifiedImportIdentifier1;
    std::string m_modifiedImportIdentifier2;

    std::map<std::string, std::string> m_configMap;

    // take 'line' and extract the (sub)string between the quotation characters
    // everything beyond the quotation is ignored!
    std::string ExtractFromQuotes(const std::string &line, const unsigned int lineNum, const Path &filePath)
    {
        std::string::size_type posFirst = line.find_first_of(m_quoteChar);
        std::string::size_type posLast = line.find_last_of(m_quoteChar);

        // if there are no quotations, fine
        if ((posFirst == std::string::npos) && (posLast == std::string::npos))
            return line;

        // if there is only one quotatation, not so nice
        if (posFirst == posLast)
        {
            std::stringstream error;
            error << "MConfig::ExtractFromQuotes( std::string line ): Only one quotation character '" << m_quoteChar
                  << "' found on line: " << lineNum << " in file: '" << filePath << "'. Please check it!";
            throw std::runtime_error(error.str());
        }

        std::string result(line.begin() + ++posFirst, line.begin() + posLast);

        // if there are more than two quotation characters, that's bad
        if (result.find(m_quoteChar) != std::string::npos)
        {
            std::stringstream error;
            error << "MConfig::ExtractFromQuotes( std::string line ): Too many quotation characters on line num: '"
                  << lineNum << "', in file: '" << filePath << "'. Only two quotation characters allowed per line!";
            throw std::runtime_error(error.str());
        }

        return result;
    }

    // find the longest key entry in the map and return its length (i.e. the number of characters)
    // only used to format the output
    int FindLongestKey()
    {
        std::map<std::string, std::string>::iterator longest = m_configMap.begin();
        for (std::map<std::string, std::string>::iterator it = m_configMap.begin(); it != m_configMap.end(); ++it)
        {
            if ((*it).first.size() > (*longest).first.size())
                longest = it;
        }

        return (*longest).first.size();
    }

    // The Init method is called everytime the ImportFile method is called.
    // ImportFile can be called from outside or if an additional config file is imported by the ScanForImports method,
    // thus Init sets m_configFilePath and m_configFileName only once!
    Path Init(Path absPath)
    {
        if (absPath.empty())
            throw std::runtime_error(
                "MConfig::Init( const string &path ): string m_configFilePath empty! Opening file failed!");
        if (!absPath.is_complete())
            absPath = m_configBasePath / absPath;
        if (!m_configInit)
        {
            m_configFilePath = absPath;
            m_configFileName = absPath.filename();

            // the path where the config file lies is the base path for imports from this config file
            // if no path has been specified
            if (m_configBasePath.empty())
                m_configBasePath = absPath.parent_path();
        }
        m_configInit = true;

        return absPath;
    }

    void ScanForImports(std::string &line, const unsigned int lineNum, const Path &filePath)
    {
        // Format: import "file" with "OLD_STRING" as "NEW_STRING"
        if (line.empty() or line.find(m_importIdentifier) == std::string::npos)
            return;

        // line = boost::trim_copy( line ); This removes only trailing and leading white spaces.
        //  Better:
        line.erase(std::unique(line.begin(), line.end(), [](char a, char b) { return a == ' ' && b == ' '; }),
                   line.end());

        std::vector<std::string> substrings = Tools::splitString<std::string>(line, false, " ");

        // check import
        if (substrings[0] == m_importIdentifier)
        {
            std::string importPath = ExtractFromQuotes(substrings[1], lineNum, filePath);
            if (substrings.size() == 2)
            {
                line.clear();
                ImportFile(importPath);
            }
            else // expect name modifier
            {
                std::string new_string = "";
                std::string old_string = "";
                if (substrings.size() != 6)
                {
                    std::stringstream error;
                    error << "MConfig::ScanForImports( std::string line, const unsigned int lineNum, const Path "
                             "&filePath ): Expected"
                          << "2 or 6 words but got " << substrings.size() << " on line: " << lineNum << " in file: '"
                          << filePath << "'. Please check it!";
                    throw std::runtime_error(error.str());
                }
                if (substrings[2] != m_modifiedImportIdentifier1)
                {
                    std::stringstream error;
                    error << "MConfig::ScanForImports( std::string line, const unsigned int lineNum, const Path "
                             "&filePath ): Expected '"
                          << m_modifiedImportIdentifier1 << "' but found '" << substrings[2] << "' on line: " << lineNum
                          << " in file: '" << filePath << "'. Please check it!";
                    throw std::runtime_error(error.str());
                }
                if (substrings[4] != m_modifiedImportIdentifier2)
                {
                    std::stringstream error;
                    error << "MConfig::ScanForImports( std::string line, const unsigned int lineNum, const Path "
                             "&filePath ): Expected '"
                          << m_modifiedImportIdentifier2 << "' but found '" << substrings[4] << "' on line: " << lineNum
                          << " in file: '" << filePath << "'. Please check it!";
                    throw std::runtime_error(error.str());
                }
                new_string = ExtractFromQuotes(substrings[5], lineNum, filePath);
                old_string = ExtractFromQuotes(substrings[3], lineNum, filePath);
                line.clear();
                ImportFile(importPath, old_string, new_string);
            }
        }
    }
};

// template function to read a map of config infos for each particle type
// The parameter objDummy is a dummy object which determines the type of the object read
// from config
template <typename confObjType>
std::map<std::string, confObjType> getConfigParticleMap(const Tools::MConfig &cfg, const std::string &label,
                                                        confObjType objDummy)
{
    std::map<std::string, confObjType> confMap = std::map<std::string, confObjType>();
    for (auto partName : Tools::getParticleTypeAbbreviations())
    {
        confMap[partName] = cfg.GetItem<confObjType>(std::string(partName) + "." + label);
    }
    return confMap;
}

// template function to read a map of config infos for each particle type
// The parameter objDummy is a dummy object which determines the type of the object read
// from config
template <typename confObjType>
std::map<std::string, confObjType> getConfigParticleReverseMap(const Tools::MConfig &cfg, const std::string &label,
                                                               confObjType objDummy)
{
    std::map<std::string, confObjType> confMap = std::map<std::string, confObjType>();
    for (auto partName : Tools::getParticleTypeAbbreviations())
    {
        confMap[cfg.GetItem<confObjType>(std::string(partName) + "." + label)] = partName;
    }
    return confMap;
}
} // namespace Tools

#endif /*TOOLS_MCONFIG*/
