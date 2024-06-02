#ifndef SCAN_HPP
#define SCAN_HPP

#include <string>

auto scan(const std::string &jsonFilePath,
          const std::string &outputDirectory,
          const int rounds,
          const int startRound,
          const std::string &shiftsFilePath,
          const std::string &lutFilePath,
          const bool is_debug) -> void;

#endif // SCAN_HPP
