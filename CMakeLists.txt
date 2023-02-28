cmake_minimum_required(VERSION 3.10.0)

project(NANOMUSIC CXX)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Look for ROOT. As long as an installation is available in the environment, this should succeed.
message(STATUS "Looking for ROOT")
find_package(ROOT REQUIRED COMPONENTS RIO)
message(STATUS "ROOT ${ROOT_VERSION} found at ${ROOT_BINDIR}")

# LCG base dir
set(LCG_BASE /cvmfs/sft.cern.ch/lcg/views/LCG_102b/x86_64-centos7-gcc12-opt)

# includes
execute_process(COMMAND root-config --incdir OUTPUT_VARIABLE ROOT_INCLUDE_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)
separate_arguments(ROOT_INCLUDE_DIR)
include_directories(
    NanoMUSiC/
    ${ROOT_INCLUDE_DIR}
    ${LCG_BASE}/include
    ${LCG_BASE}/include/rapidjson
    ${CMAKE_SOURCE_DIR}/NanoMUSiC/MUSiC/external/range-v3/include
    ${CMAKE_SOURCE_DIR}/NanoMUSiC/MUSiC/external/
    ${CMAKE_SOURCE_DIR}/NanoMUSiC/MUSiC/include/
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

# # correctionlib compilation and linking flags
# execute_process(COMMAND correction config --cflags --ldflags --rpath OUTPUT_VARIABLE CORRECTIONLIB_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
# separate_arguments(CORRECTIONLIB_FLAGS)
#
# # compilation options
# execute_process(COMMAND root-config --cflags OUTPUT_VARIABLE ROOT_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
# separate_arguments(ROOT_CFLAGS)
# add_compile_options(${ROOT_CFLAGS} ${CORRECTIONLIB_FLAGS} -fdiagnostics-color=always -MD -MP -O3 -Wall -pthread -m64 -ffloat-store)
#
# # root linker options
# execute_process(COMMAND root-config --ldflags OUTPUT_VARIABLE ROOT_LDFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
# separate_arguments(ROOT_LDFLAGS)
#
# # root libs
# execute_process(COMMAND root-config --libs OUTPUT_VARIABLE ROOT_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)
# separate_arguments(ROOT_LIBS)
#
# add_link_options(
# -fdiagnostics-color=always
# ${ROOT_LDFLAGS}
# ${ROOT_LIBS}
# ${CORRECTIONLIB_FLAGS}
# -L${LCG_BASE}/lib
# -L${LCG_BASE}/lib64
# -ldcap
# -lgsl
# -lgslcblas
# -lboost_system
# -lboost_thread
# -lboost_filesystem
# -lboost_date_time
# -lboost_serialization
# -lboost_program_options
# -lLHAPDF
# -lfmt
# -L${CMAKE_SOURCE_DIR}/NanoMUSiC/MUSiC-Utils/lib
# -O3
# -lz
# -lstdc++fs
# )

# base library
add_subdirectory(NanoMUSiC)

# enable tests
enable_testing()

# list(APPEND CMAKE_CTEST_ARGUMENTS "--output-on-failure")