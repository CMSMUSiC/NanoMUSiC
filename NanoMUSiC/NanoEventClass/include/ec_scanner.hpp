#ifndef EC_SCANNER_HPP
#define EC_SCANNER_HPP

#include <cstdlib>
#include <filesystem>
#include <glob.h> // glob(), globfree()
#include <limits>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string.h> // memset()
#include <string>
#include <vector>

#include "fmt/format.h"
#include "toml.hpp"

#include "Distribution.hpp"
#include "NanoEventClass.hpp"
#include "Scanner.hpp"
#include "distribution_factory.hpp"

#include "TDirectory.h"
#include "TH1.h"
using namespace ROOT;

inline auto get_source_files(const std::string &path,
                             const std::string &year_pattern,
                             std::optional<std::size_t> limit = std::nullopt) -> std::vector<std::string>
{
    fmt::print("{}\n", path);
    fmt::print("{}\n", year_pattern);
    // glob struct resides on the stack
    glob_t glob_result;
    memset(&glob_result, 0, sizeof(glob_result));

    // do the glob operation
    int return_value = glob(fmt::format("{}/*{}.root", path, year_pattern).c_str(), GLOB_TILDE, NULL, &glob_result);
    if (return_value != 0)
    {
        globfree(&glob_result);
        std::stringstream ss;
        ss << "glob() failed with return_value " << return_value << std::endl;
        throw std::runtime_error(ss.str());
    }

    auto file_limit = std::numeric_limits<std::size_t>::max();
    if (limit)
    {
        file_limit = *limit;
        fmt::print("\n\n");
        fmt::print("🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴\n");
        fmt::print("🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴\n");
        fmt::print("WARNING: Running in DEBUG MODE. Loading a limited number of input files.\n");
        fmt::print("🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴\n");
        fmt::print("🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴\n");
        fmt::print("\n\n");
    }

    // collect all the filenames into a std::vector<std::string>
    std::vector<std::string> filenames;
    for (std::size_t i = 0; i < glob_result.gl_pathc and i < file_limit; ++i)
    {
        auto file_path = std::string(glob_result.gl_pathv[i]);
        if (file_path.find("cutflow") == std::string::npos)
        {
            filenames.push_back(std::string(glob_result.gl_pathv[i]));
        }
    }

    // cleanup
    globfree(&glob_result);

    // done
    return filenames;
}

struct Arguments
{
    std::string histograms_dir;
    std::vector<std::string> years;
    std::vector<std::string> classes;
};

auto arg_parse(int argc, char *argv[]) -> Arguments
{

    if (argc < 2)
    {
        fmt::print(stderr, "ERROR: Could not start scanner. Input path not provided.\n");
        fmt::print(
            "\nUSAGE: ec_scanner SCANNER_CONFIG_FILE\n"
            "-- SCANNER_CONFIG_FILE: path to scanner config file (.toml)\n\n");
        std::exit(EXIT_FAILURE);
    }

    toml::table args_toml;
    try
    {
        args_toml = toml::parse_file(argv[1]);
    }
    catch (const toml::parse_error &err)
    {
        fmt::print(stderr, "ERROR: Coudl not parse scanner config file.\n");
        std::exit(EXIT_FAILURE);
    }

    auto args = Arguments{.histograms_dir = args_toml["histograms_dir"].value_or(""), .years = {}, .classes = {}};

    for (auto &&elem : *args_toml.get_as<toml::array>("years"))
    {
        elem.visit(
            [&args](auto &&el) noexcept
            {
                // if  (toml::is_string<decltype(el)>)
                // {
                args.years.push_back(el.value_or(""));
                // }
                // else
                // {
                //     fmt::print(stderr, "ERROR: Could not read list of years. Element \"{}\" is not a string.\n", el);
                // }
            });
    }

    for (auto &&elem : *args_toml.get_as<toml::array>("classes"))
    {
        elem.visit(
            [&args](auto &&el) noexcept
            {
                // if constexpr (toml::is_string<decltype(el)>)
                // {
                args.classes.push_back(el.value_or(""));
                // }
                // else
                // {
                //     fmt::print(stderr, "ERROR: Could not read list of classes. Element \"{}\" is not a string.\n",
                //     el);
                // }
            });
    }
    if (not(std::filesystem::is_directory(args.histograms_dir)))
    {
        fmt::print(stderr, "ERROR: Could not find input directory {}.\n", args.histograms_dir);
        std::exit(EXIT_FAILURE);
    }

    fmt::print(
        "Args: {} - [{}] - [{}]\n", args.histograms_dir, fmt::join(args.years, ", "), fmt::join(args.classes, ", "));
    return args;
}
#endif // EC_SCANNER_HPP
