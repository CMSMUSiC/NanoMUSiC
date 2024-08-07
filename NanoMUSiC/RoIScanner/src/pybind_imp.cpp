#include "scan.hpp"

#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include <cstdlib>
#include <pybind11/pytypes.h>
// namespace py = pybind11;
using namespace pybind11::literals;

PYBIND11_MODULE(scanner_imp, m)
{
    m.def("scan",
          &scan,
          "json_file_path"_a,
          "output_directory"_a,
          "rounds"_a,
          "start_round"_a,
          "shifts_file_path"_a,
          "lut_file_path"_a,
          "scan_type"_a,
          "is_debug"_a = false,
          "Entry point for the RoIScanner code.");
    m.doc() = "python bindings for the RoIScanner";
}
