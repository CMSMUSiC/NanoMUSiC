# correctionlib compilation and linking flags
execute_process(COMMAND correction config --cflags --ldflags --rpath OUTPUT_VARIABLE CORRECTIONLIB_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
separate_arguments(CORRECTIONLIB_FLAGS)

# compilation options
execute_process(COMMAND root-config --cflags OUTPUT_VARIABLE ROOT_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
separate_arguments(ROOT_CFLAGS)
add_compile_options(-g)

# add_compile_options(-I/cvmfs/sft.cern.ch/lcg/views/LCG_102b/x86_64-centos7-gcc12-opt/include)
add_compile_options(-I/cvmfs/sft.cern.ch/lcg/views/LCG_102b/x86_64-centos7-gcc11-opt/include)

add_compile_options(${ROOT_CFLAGS} ${CORRECTIONLIB_FLAGS} -fdiagnostics-color=always -MD -MP -O3 -Wall -march=native -pthread -m64 -ffloat-store)

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
    ${CORRECTIONLIB_FLAGS}
    -lLHAPDF
    -lfmt
    -lz
    -lstdc++fs
)

add_executable(
    heavy_validation //
    src/main.cpp //
    ${CMAKE_SOURCE_DIR}/NanoMUSiC/MUSiC/src/TOMLConfig.cpp //
    ${CMAKE_SOURCE_DIR}/NanoMUSiC/MUSiC/src/BinLimits.cpp //
    ${CMAKE_SOURCE_DIR}/NanoMUSiC/MUSiC/src/NanoObjects.cpp //
    ${CMAKE_SOURCE_DIR}/NanoMUSiC/MUSiC/src/JetCorrector.cpp //
    ${CMAKE_SOURCE_DIR}/NanoMUSiC/NanoEventClass/src/NanoEventClass.cpp //
    src/ZToLepLepX.cpp //

    # src/Dijets.cpp //
)
target_link_libraries(heavy_validation LINK_PUBLIC ROOT::RIO ROOT::Net)
install(TARGETS heavy_validation DESTINATION ${CMAKE_SOURCE_DIR}/bin)
