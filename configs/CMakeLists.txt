execute_process(COMMAND root-config --cflags OUTPUT_VARIABLE ROOT_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
separate_arguments(ROOT_CFLAGS)
add_compile_options(-g)

add_compile_options(-I/cvmfs/sft.cern.ch/lcg/views/LCG_106/x86_64-el9-gcc13-opt/include)

add_compile_options(${ROOT_CFLAGS} -fdiagnostics-color=always -MD -MP -O3 -Wall -march=native -pthread -m64 -ffloat-store)

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
    # ${CORRECTIONLIB_FLAGS}
    -lLHAPDF
    -lfmt
    -lz
    -lstdc++fs
    -L/cvmfs/sft.cern.ch/lcg/views/LCG_106/x86_64-el9-gcc13-opt/lib
    -L/cvmfs/sft.cern.ch/lcg/views/LCG_106/x86_64-el9-gcc13-opt/lib64
) 


add_library(ConfigParams SHARED
	src/ConfigParams.cpp 
)
target_include_directories(ConfigParams PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(ConfigParams LINK_PUBLIC ROOT::RIO ROOT::Net)
install(TARGETS ConfigParams DESTINATION ${CMAKE_SOURCE_DIR}/lib)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libConfigParams_rdict.pcm DESTINATION ${CMAKE_SOURCE_DIR}/lib)


message(STATUS "Generating ConfigParams dictionaries ...")
ROOT_GENERATE_DICTIONARY(ConfigParamsDict include/ConfigParams.hpp MODULE ConfigParams LINKDEF include/ConfigParamsLinkDef.h)
message(STATUS "Done.")


