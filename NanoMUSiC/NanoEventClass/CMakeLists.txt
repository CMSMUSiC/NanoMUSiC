 # root linker options
execute_process(COMMAND root-config --ldflags OUTPUT_VARIABLE ROOT_LDFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
separate_arguments(ROOT_LDFLAGS)

# root libs
execute_process(COMMAND root-config --libs OUTPUT_VARIABLE ROOT_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)
separate_arguments(ROOT_LIBS)

add_link_options(
    -fdiagnostics-color=always
    ${ROOT_LDFLAGS}
    ${ROOT_LIBS}
    -L${LCG_BASE}/lib
    -L${LCG_BASE}/lib64
    -lfmt
    -lz
    -fno-associative-math
    -lgsl
    -lgslcblas
    -lm
)
add_compile_options(-I/cvmfs/sft.cern.ch/lcg/views/LCG_104c/x86_64-el9-gcc13-opt/include)

add_library(NanoEventClass SHARED
        src/NanoEventClass.cpp
        src/Scanner.cpp
        src/Distribution.cpp
        src/distribution_factory.cpp
        src/ConvolutionComputer.cpp
        src/ConvolutionLookup.cpp
)

# compilation options
execute_process(COMMAND root-config --cflags OUTPUT_VARIABLE ROOT_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
separate_arguments(ROOT_CFLAGS)


target_include_directories(NanoEventClass PUBLIC 
        ${CMAKE_SOURCE_DIR}/NanoMUSiC/NanoEventClass/include
        ${CMAKE_SOURCE_DIR}/NanoMUSiC/MUSiC/external
  )
target_link_libraries(NanoEventClass ROOT::RIO ROOT::Net ROOT::ROOTVecOps fmt gsl gslcblas)

target_include_directories(NanoEventClass PUBLIC ./)

target_compile_options(NanoEventClass PUBLIC
        -MD
        -MP
        -O3
        -Wall
        -Wextra
        -fPIC
        -pthread
        -std=c++17
        -m64
        -g
        -fdiagnostics-color=always 
        -march=native 
        -ffloat-store
        ${ROOT_CFLAGS}
)

# Install the library
install(TARGETS NanoEventClass
        LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/lib
        ARCHIVE DESTINATION ${CMAKE_SOURCE_DIR}/lib
)

message(STATUS "Generating NanoEventClass dictionaries ...")
ROOT_GENERATE_DICTIONARY(NanoEventClassDict include/NanoEventClass.hpp include/Distribution.hpp include/distribution_factory.hpp include/Scanner.hpp MODULE NanoEventClass LINKDEF include/NanoEventClassLinkDef.h)
message(STATUS "Done.")

install(TARGETS NanoEventClassDict DESTINATION ${CMAKE_SOURCE_DIR}/lib)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libNanoEventClass_rdict.pcm DESTINATION ${CMAKE_SOURCE_DIR}/lib)
install(TARGETS NanoEventClass DESTINATION ${CMAKE_SOURCE_DIR}/lib)


# ec_scanner
add_executable(ec_scanner
        src/NanoEventClass.cpp
        src/Scanner.cpp
        src/Distribution.cpp
        src/distribution_factory.cpp
        src/ConvolutionComputer.cpp
        src/ConvolutionLookup.cpp
        src/main/ec_scanner.cpp)

target_compile_options(ec_scanner PUBLIC 
        -MD
        -MP
        -O3
        -Wall
        -Wextra
        -fPIC
        -pthread
        -std=c++17
        -m64
        -g
        -fdiagnostics-color=always 
        -march=native 
        -ffloat-store
        ${ROOT_CFLAGS}
        )
target_link_libraries(NanoEventClass ROOT::RIO ROOT::Net ROOT::ROOTVecOps fmt gsl gslcblas)

target_include_directories(ec_scanner PUBLIC 
        ${CMAKE_SOURCE_DIR}/NanoMUSiC/NanoEventClass/include
        ${CMAKE_SOURCE_DIR}/NanoMUSiC/MUSiC/external
  )

install(TARGETS ec_scanner DESTINATION ${CMAKE_SOURCE_DIR}/bin)
