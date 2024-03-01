# compilation options
execute_process(COMMAND root-config --cflags OUTPUT_VARIABLE ROOT_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
separate_arguments(ROOT_CFLAGS)
add_compile_options(-g)

add_compile_options(-I/cvmfs/sft.cern.ch/lcg/views/LCG_105a/x86_64-el9-gcc12-opt/include)

add_compile_options(${ROOT_CFLAGS} -Wall -Wextra -g -O3 -ffast-math -fPIC -fdiagnostics-color=always -MD -MP -O3 -Wall -march=native -pthread -m64 -ffloat-store)

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
        -lstdc++fs
        -L/cvmfs/sft.cern.ch/lcg/views/LCG_105a/x86_64-el9-gcc12-opt/lib
        -L/cvmfs/sft.cern.ch/lcg/views/LCG_105a/x86_64-el9-gcc12-opt/lib64
        -lgsl
        -lgslcblas
        -lm
        -lboost_program_options
)

add_executable(createLookupTable
        src/ConvolutionComputer.cpp
        src/ConvolutionLookup.cpp
        src/createLookupTable.cpp)

target_compile_options(createLookupTable PUBLIC -Wall -Wextra -std=c++17 -g -O3 -ffast-math -MD -MP -fPIC -pthread)

target_include_directories(createLookupTable PUBLIC include ${CMAKE_SOURCE_DIR}/NanoMUSiC/NanoEventClass/include/external)

install(TARGETS createLookupTable DESTINATION ${CMAKE_SOURCE_DIR}/bin)

# build lookuptable
add_custom_command(
        TARGET createLookupTable POST_BUILD
        COMMAND cmake -E cmake_echo_color --yellow --bold ""
        COMMAND cmake -E cmake_echo_color --yellow --bold "=================================================="
        COMMAND cmake -E cmake_echo_color --yellow --bold " Before scanning, do not forget to build the LUT:"
        COMMAND cmake -E cmake_echo_color --yellow --bold " ninja lut"
        COMMAND cmake -E cmake_echo_color --yellow --bold "=================================================="
        COMMAND cmake -E cmake_echo_color --yellow --bold ""
)

# build lookuptable
add_custom_command(
        OUTPUT ${CMAKE_SOURCE_DIR}/bin/lookuptable.bin
        DEPENDS createLookupTable
        COMMAND createLookupTable ${CMAKE_SOURCE_DIR}/bin/lookuptable.bin
)

add_custom_target(lut
        DEPENDS ${CMAKE_SOURCE_DIR}/bin/lookuptable.bin
)

add_executable(scanClass
        src/ConvolutionComputer.cpp
        src/ECScanner.cpp
        src/Dicer.cpp
        src/MCBin.cpp
        src/ScanResult.cpp
        src/scanClass.cpp
        src/util.cpp
        src/ConvolutionLookup.cpp)

target_compile_options(scanClass PUBLIC -Wall -Wextra -std=c++17 -g -O3 -ffast-math -MD -MP -fPIC -pthread)

target_include_directories(scanClass PUBLIC include  ${CMAKE_SOURCE_DIR}/NanoMUSiC/NanoEventClass/include/external)
install(TARGETS scanClass DESTINATION ${CMAKE_SOURCE_DIR}/bin)

add_executable(pCalc
        src/ConvolutionComputer.cpp
        src/ConvolutionLookup.cpp
        src/pCalc.cpp)

target_compile_options(pCalc PUBLIC -Wall -Wextra -std=c++17 -g -O3 -ffast-math -MD -MP -fPIC -pthread)
target_include_directories(pCalc PUBLIC include  ${CMAKE_SOURCE_DIR}/NanoMUSiC/NanoEventClass/include/external)
install(TARGETS pCalc DESTINATION ${CMAKE_SOURCE_DIR}/bin)

add_library(ConvolutionComputer SHARED
        src/ConvolutionComputer.cpp
)
target_compile_options(ConvolutionComputer PUBLIC -Wall -Wextra -std=c++17 -g -O3 -ffast-math -MD -MP -fPIC -pthread)
target_include_directories(ConvolutionComputer PUBLIC include  ${CMAKE_SOURCE_DIR}/NanoMUSiC/NanoEventClass/include/external)
install(TARGETS ConvolutionComputer DESTINATION ${CMAKE_SOURCE_DIR}/lib)

add_library(ConvolutionLookup SHARED
        src/ConvolutionLookup.cpp
        src/ConvolutionComputer.cpp
)
target_compile_options(ConvolutionLookup PUBLIC -Wall -Wextra -std=c++17 -g -O3 -ffast-math -MD -MP -fPIC -pthread)
target_include_directories(ConvolutionLookup PUBLIC include  ${CMAKE_SOURCE_DIR}/NanoMUSiC/NanoEventClass/include/external)
install(TARGETS ConvolutionLookup DESTINATION ${CMAKE_SOURCE_DIR}/lib)
