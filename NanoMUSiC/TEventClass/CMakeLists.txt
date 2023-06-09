add_library(TEventClass SHARED
        src/TEventClass.cpp
)

target_include_directories(TEventClass PUBLIC ./)
target_compile_options(TEventClass PUBLIC
        -MD
        -MP
        -O3
        -Wall
        -fPIC

        # -fsignaling-nans
        # -funsafe-math-optimizations
        # -fno-rounding-math
        # -fno-signaling-nans
        # -fcx-limited-range
        # -fno-associative-math
        -pthread
        -std=c++17
        -m64
        -g
        -ffloat-store
)

target_include_directories(TEventClass PUBLIC ${CMAKE_SOURCE_DIR}/NanoMUSiC/TEventClass/include)

target_link_options(TEventClass PRIVATE -lz -fno-associative-math)

message(STATUS "Generating TEventClass dictionaries ...")
ROOT_GENERATE_DICTIONARY(TEventClassDict include/TEventClass.hpp MODULE TEventClass LINKDEF include/TEventClassLinkDef.h)
message(STATUS "Done.")

install(TARGETS TEventClassDict DESTINATION ${CMAKE_SOURCE_DIR}/lib)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libTEventClass_rdict.pcm DESTINATION ${CMAKE_SOURCE_DIR}/lib)
install(TARGETS TEventClass DESTINATION ${CMAKE_SOURCE_DIR}/lib)
