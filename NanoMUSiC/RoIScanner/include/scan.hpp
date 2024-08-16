#ifndef SCAN_HPP
#define SCAN_HPP

#include <string>

auto scan(const std::string &jsonFilePath,
          const std::string &outputDirectory,
          const int rounds,
          const int start_round,
          const std::string &shiftsFilePath,
          const std::string &lutFilePath,
          const std::string &scanType,
          const bool is_debug) -> bool;

#endif // SCAN_HPP
