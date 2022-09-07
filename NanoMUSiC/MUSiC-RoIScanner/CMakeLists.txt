add_executable(createLookupTable
        src/ConvolutionComputer.cc
        src/ConvolutionLookup.cc
        src/createLookupTable.cc)
target_compile_options(createLookupTable PUBLIC -Wall -Wextra -std=c++17 -g -O3 -ffast-math -MD -MP -fPIC -pthread)
target_link_options(createLookupTable PUBLIC -Wall -Wextra -std=c++17 -g -O3 -ffast-math -MD -MP -fPIC -pthread)
target_include_directories(createLookupTable PUBLIC include include/rapidjson)
install(TARGETS createLookupTable DESTINATION ${CMAKE_SOURCE_DIR}/bin)

# build lookuptable
add_custom_command(
        OUTPUT lookuptable.bin
        DEPENDS createLookupTable
        COMMAND createLookupTable lookuptable.bin
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lookuptable.bin DESTINATION ${CMAKE_SOURCE_DIR}/bin)

add_custom_target(lookuptable_bin ALL
        DEPENDS lookuptable.bin
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
target_include_directories(scanClass PUBLIC include include/rapidjson)
install(TARGETS scanClass DESTINATION ${CMAKE_SOURCE_DIR}/bin)

add_executable(pCalc
        src/ConvolutionComputer.cc
        src/ConvolutionLookup.cc
        src/pCalc.cc)
target_compile_options(pCalc PUBLIC -Wall -Wextra -std=c++17 -g -O3 -ffast-math -MD -MP -fPIC -pthread)
target_link_options(pCalc PUBLIC -Wall -Wextra -std=c++17 -g -O3 -ffast-math -MD -MP -fPIC -pthread)
target_include_directories(pCalc PUBLIC include include/rapidjson)
install(TARGETS pCalc DESTINATION ${CMAKE_SOURCE_DIR}/bin)

add_library(ConvolutionComputer SHARED
        src/ConvolutionComputer.cc
)
target_compile_options(ConvolutionComputer PUBLIC -Wall -Wextra -std=c++17 -g -O3 -ffast-math -MD -MP -fPIC -pthread)
target_link_options(ConvolutionComputer PUBLIC -Wall -Wextra -std=c++17 -g -O3 -ffast-math -MD -MP -fPIC -pthread)
target_include_directories(ConvolutionComputer PUBLIC include include/rapidjson)
install(TARGETS ConvolutionComputer DESTINATION ${CMAKE_SOURCE_DIR}/lib)

add_library(ConvolutionLookup SHARED
        src/ConvolutionLookup.cc
        src/ConvolutionComputer.cc
)
target_compile_options(ConvolutionLookup PUBLIC -Wall -Wextra -std=c++17 -g -O3 -ffast-math -MD -MP -fPIC -pthread)
target_link_options(ConvolutionLookup PUBLIC -Wall -Wextra -std=c++17 -g -O3 -ffast-math -MD -MP -fPIC -pthread)
target_include_directories(ConvolutionLookup PUBLIC include include/rapidjson)
install(TARGETS ConvolutionLookup DESTINATION ${CMAKE_SOURCE_DIR}/lib)
