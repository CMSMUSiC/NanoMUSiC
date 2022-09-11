add_executable(createLookupTable
        src/ConvolutionComputer.cc
        src/ConvolutionLookup.cc
        src/createLookupTable.cc)
target_compile_options(createLookupTable PUBLIC -Wall -Wextra -std=c++17 -g -O3 -ffast-math -MD -MP -fPIC -pthread)
target_link_options(createLookupTable PUBLIC -Wall -Wextra -std=c++17 -g -O3 -ffast-math -MD -MP -fPIC -pthread)
target_include_directories(createLookupTable PUBLIC include)
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
        src/ConvolutionComputer.cc
        src/ECScanner.cc
        src/Dicer.cc
        src/MCBin.cc
        src/ScanResult.cc
        src/scanClass.cc
        src/util.cc
        src/ConvolutionLookup.cc)
target_compile_options(scanClass PUBLIC -Wall -Wextra -std=c++17 -g -O3 -ffast-math -MD -MP -fPIC -pthread)
target_link_options(scanClass PUBLIC -Wall -Wextra -std=c++17 -g -O3 -ffast-math -MD -MP -fPIC -pthread)
target_include_directories(scanClass PUBLIC include)
install(TARGETS scanClass DESTINATION ${CMAKE_SOURCE_DIR}/bin)

add_executable(pCalc
        src/ConvolutionComputer.cc
        src/ConvolutionLookup.cc
        src/pCalc.cc)
target_compile_options(pCalc PUBLIC -Wall -Wextra -std=c++17 -g -O3 -ffast-math -MD -MP -fPIC -pthread)
target_link_options(pCalc PUBLIC -Wall -Wextra -std=c++17 -g -O3 -ffast-math -MD -MP -fPIC -pthread)
target_include_directories(pCalc PUBLIC include)
install(TARGETS pCalc DESTINATION ${CMAKE_SOURCE_DIR}/bin)

add_library(ConvolutionComputer SHARED
        src/ConvolutionComputer.cc
)
target_compile_options(ConvolutionComputer PUBLIC -Wall -Wextra -std=c++17 -g -O3 -ffast-math -MD -MP -fPIC -pthread)
target_link_options(ConvolutionComputer PUBLIC -Wall -Wextra -std=c++17 -g -O3 -ffast-math -MD -MP -fPIC -pthread)
target_include_directories(ConvolutionComputer PUBLIC include)
install(TARGETS ConvolutionComputer DESTINATION ${CMAKE_SOURCE_DIR}/lib)

add_library(ConvolutionLookup SHARED
        src/ConvolutionLookup.cc
        src/ConvolutionComputer.cc
)
target_compile_options(ConvolutionLookup PUBLIC -Wall -Wextra -std=c++17 -g -O3 -ffast-math -MD -MP -fPIC -pthread)
target_link_options(ConvolutionLookup PUBLIC -Wall -Wextra -std=c++17 -g -O3 -ffast-math -MD -MP -fPIC -pthread)
target_include_directories(ConvolutionLookup PUBLIC include)
install(TARGETS ConvolutionLookup DESTINATION ${CMAKE_SOURCE_DIR}/lib)
