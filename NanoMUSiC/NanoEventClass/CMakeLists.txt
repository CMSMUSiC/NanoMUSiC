add_library(NanoEventClass SHARED
        src/NanoEventClass.cpp
)

target_include_directories(NanoEventClass PUBLIC ./)
target_compile_options(NanoEventClass PUBLIC
        -MD
        -MP
        -O3
        -Wall
        -fPIC
        -pthread
        -std=c++17
        -m64
        -g
)

target_include_directories(NanoEventClass PUBLIC ${CMAKE_SOURCE_DIR}/NanoMUSiC/NanoEventClass/include)

# Install the library
install(TARGETS NanoEventClass
        LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/lib
        ARCHIVE DESTINATION ${CMAKE_SOURCE_DIR}/lib
)

install(TARGETS NanoEventClass DESTINATION ${CMAKE_SOURCE_DIR}/lib)

# install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libNanoEventClass_rdict.pcm DESTINATION ${CMAKE_SOURCE_DIR}/lib)
install(TARGETS NanoEventClass DESTINATION ${CMAKE_SOURCE_DIR}/lib)
