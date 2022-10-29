add_executable(createLookupTable
        src/ConvolutionComputer.cpp
        src/ConvolutionLookup.cpp
        src/createLookupTable.cpp)
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
        src/ConvolutionComputer.cpp
        src/ECScanner.cpp
        src/Dicer.cpp
        src/MCBin.cpp
        src/ScanResult.cpp
        src/scanClass.cpp
        src/util.cpp
        src/ConvolutionLookup.cpp)
target_compile_options(scanClass PUBLIC -Wall -Wextra -std=c++17 -g -O3 -ffast-math -MD -MP -fPIC -pthread)
target_link_options(scanClass PUBLIC -Wall -Wextra -std=c++17 -g -O3 -ffast-math -MD -MP -fPIC -pthread)
target_include_directories(scanClass PUBLIC include)
install(TARGETS scanClass DESTINATION ${CMAKE_SOURCE_DIR}/bin)

add_executable(pCalc
        src/ConvolutionComputer.cpp
        src/ConvolutionLookup.cpp
        src/pCalc.cpp)
target_compile_options(pCalc PUBLIC -Wall -Wextra -std=c++17 -g -O3 -ffast-math -MD -MP -fPIC -pthread)
target_link_options(pCalc PUBLIC -Wall -Wextra -std=c++17 -g -O3 -ffast-math -MD -MP -fPIC -pthread)
target_include_directories(pCalc PUBLIC include)
install(TARGETS pCalc DESTINATION ${CMAKE_SOURCE_DIR}/bin)

add_library(ConvolutionComputer SHARED
        src/ConvolutionComputer.cpp
)
target_compile_options(ConvolutionComputer PUBLIC -Wall -Wextra -std=c++17 -g -O3 -ffast-math -MD -MP -fPIC -pthread)
target_link_options(ConvolutionComputer PUBLIC -Wall -Wextra -std=c++17 -g -O3 -ffast-math -MD -MP -fPIC -pthread)
target_include_directories(ConvolutionComputer PUBLIC include)
install(TARGETS ConvolutionComputer DESTINATION ${CMAKE_SOURCE_DIR}/lib)

add_library(ConvolutionLookup SHARED
        src/ConvolutionLookup.cpp
        src/ConvolutionComputer.cpp
)
target_compile_options(ConvolutionLookup PUBLIC -Wall -Wextra -std=c++17 -g -O3 -ffast-math -MD -MP -fPIC -pthread)
target_link_options(ConvolutionLookup PUBLIC -Wall -Wextra -std=c++17 -g -O3 -ffast-math -MD -MP -fPIC -pthread)
target_include_directories(ConvolutionLookup PUBLIC include)
install(TARGETS ConvolutionLookup DESTINATION ${CMAKE_SOURCE_DIR}/lib)
