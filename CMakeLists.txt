cmake_minimum_required(VERSION 3.14.0)
project(NANOMUSIC CXX)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Look for ROOT. As long as an installation is available in the environment, this should succeed.
message(STATUS "Looking for ROOT")
find_package(ROOT REQUIRED COMPONENTS RIO)
message(STATUS "ROOT ${ROOT_VERSION} found at ${ROOT_BINDIR}")

# LCG base dir
set(LCG_BASE /cvmfs/sft.cern.ch/lcg/views/LCG_102/x86_64-centos7-gcc11-opt)

# includes
execute_process(COMMAND root-config --incdir OUTPUT_VARIABLE ROOT_INCLUDE_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)
separate_arguments(ROOT_INCLUDE_DIR)
include_directories(
    NanoMUSiC/
    ${ROOT_INCLUDE_DIR}
    ${LCG_BASE}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/NanoMUSiC/PxlAnalyzer
    ${CMAKE_CURRENT_SOURCE_DIR}/NanoMUSiC/PxlAnalyzer/Pxl/Pxl/src
    ${CMAKE_CURRENT_SOURCE_DIR}/NanoMUSiC/PxlAnalyzer/Tools
    ${CMAKE_CURRENT_SOURCE_DIR}/NanoMUSiC/PxlAnalyzer/include
    ${CMAKE_CURRENT_SOURCE_DIR}/NanoMUSiC/tools/include
    ${CMAKE_CURRENT_SOURCE_DIR}/NanoMUSiC/PxlAnalyzer/Tools/
    ${CMAKE_CURRENT_SOURCE_DIR}/NanoMUSiC/PxlAnalyzer/Tools/PXL/
    ${CMAKE_CURRENT_SOURCE_DIR}/NanoMUSiC/PxlAnalyzer/Main
    ${CMAKE_CURRENT_SOURCE_DIR}/NanoMUSiC/MUSiC-Utils/include
    ${CMAKE_CURRENT_SOURCE_DIR}/correctionlib/correctionlib/include
)

# ${LCG_BASE}/include/boost
# ${LCG_BASE}/include/gsl

# compilation options
execute_process(COMMAND root-config --cflags OUTPUT_VARIABLE ROOT_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
separate_arguments(ROOT_CFLAGS)
add_compile_options(${ROOT_CFLAGS} -MD -MP -O3 -Wall -fconcepts-ts -pthread -m64 -ffloat-store)

# root linker options
execute_process(COMMAND root-config --ldflags OUTPUT_VARIABLE ROOT_LDFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
separate_arguments(ROOT_LDFLAGS)

# root libs
execute_process(COMMAND root-config --libs --glibs OUTPUT_VARIABLE ROOT_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)
separate_arguments(ROOT_LIBS)

add_link_options(
    ${ROOT_LDFLAGS}
    ${ROOT_LIBS}
    -L${LCG_BASE}/lib
    -L${LCG_BASE}/lib64
    -ldcap
    -lgsl
    -lgslcblas
    -lboost_system
    -lboost_thread
    -lboost_filesystem
    -lboost_date_time
    -lboost_serialization
    -lboost_program_options
    -lLHAPDF
    -L${CMAKE_CURRENT_SOURCE_DIR}/NanoMUSiC/MUSiC-Utils/lib
    -L${CMAKE_CURRENT_SOURCE_DIR}/NanoMUSiC/correctionlib/correctionlib/lib
    -lcorrectionlib
    -O3fast
    -lz
    -lstdc++fs
)

# base library
add_subdirectory(NanoMUSiC)

# enable tests
enable_testing()
