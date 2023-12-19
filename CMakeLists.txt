cmake_minimum_required(VERSION 3.10.0)

project(NANOMUSIC CXX)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Look for ROOT.
# As long as an installation is available in the environment, this should succeed.
message(STATUS "Looking for ROOT")
find_package(ROOT REQUIRED COMPONENTS RIO)
message(STATUS "ROOT ${ROOT_VERSION} found at ${ROOT_BINDIR}")

# LCG base dir
set(LCG_BASE /cvmfs/sft.cern.ch/lcg/views/LCG_104c/x86_64-el9-gcc13-opt)

# includes
execute_process(COMMAND root-config --incdir OUTPUT_VARIABLE ROOT_INCLUDE_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)
separate_arguments(ROOT_INCLUDE_DIR)
include_directories(
    NanoMUSiC/
    ${ROOT_INCLUDE_DIR}
    ${LCG_BASE}/include
    ${LCG_BASE}/include/rapidjson
    ${CMAKE_SOURCE_DIR}/NanoMUSiC/TEventClass/include/
    ${CMAKE_SOURCE_DIR}/NanoMUSiC/NanoEventClass/include/
    ${CMAKE_SOURCE_DIR}/NanoMUSiC/MUSiC/external/
    ${CMAKE_SOURCE_DIR}/NanoMUSiC/MUSiC/include/
    ${CMAKE_SOURCE_DIR}/NanoMUSiC/MUSiC-LightSkimmer/include/
    ${CMAKE_SOURCE_DIR}/NanoMUSiC/MUSiC-Classification/include/
    ${CMAKE_SOURCE_DIR}/NanoMUSiC/MUSiC-Validation/include/
    ${CMAKE_SOURCE_DIR}/NanoMUSiC/PxlAnalyzer
    ${CMAKE_SOURCE_DIR}/NanoMUSiC/PxlAnalyzer
    ${CMAKE_SOURCE_DIR}/NanoMUSiC/PxlAnalyzer/Pxl/Pxl/src
    ${CMAKE_SOURCE_DIR}/NanoMUSiC/PxlAnalyzer/Tools
    ${CMAKE_SOURCE_DIR}/NanoMUSiC/PxlAnalyzer/include
    ${CMAKE_SOURCE_DIR}/NanoMUSiC/tools/include
    ${CMAKE_SOURCE_DIR}/NanoMUSiC/PxlAnalyzer/Tools/
    ${CMAKE_SOURCE_DIR}/NanoMUSiC/PxlAnalyzer/Tools/PXL/
    ${CMAKE_SOURCE_DIR}/NanoMUSiC/PxlAnalyzer/Main
    ${CMAKE_SOURCE_DIR}/NanoMUSiC/MUSiC-Utils/include
)

# base library
add_subdirectory(NanoMUSiC)

# enable tests
# enable_testing()

# list(APPEND CMAKE_CTEST_ARGUMENTS "--output-on-failure")
