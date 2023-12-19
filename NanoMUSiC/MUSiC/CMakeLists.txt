# ################################################
# ################################################
# NanoAODReader
# ################################################
# ################################################

add_library(NanoAODReader
    src/NanoAODReader.cpp
)
target_compile_options(NanoAODReader PUBLIC -g -c -O3)

# #####################################################
# #####################################################
# NanoMUSiC (Skimmer)
# #####################################################
# #####################################################

# correctionlib compilation and linking flags
execute_process(COMMAND correction config --cflags --ldflags --rpath OUTPUT_VARIABLE CORRECTIONLIB_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
separate_arguments(CORRECTIONLIB_FLAGS)

# compilation options
execute_process(COMMAND root-config --cflags OUTPUT_VARIABLE ROOT_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
separate_arguments(ROOT_CFLAGS)
add_compile_options(-g)

add_compile_options(-I/cvmfs/sft.cern.ch/lcg/views/LCG_104/x86_64-el9-gcc12-opt/include)

add_compile_options(${ROOT_CFLAGS} ${CORRECTIONLIB_FLAGS} -fdiagnostics-color=always -MD -MP -O3 -Wall -pthread -m64 -ffloat-store)

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
    ${CORRECTIONLIB_FLAGS}
    -L${LCG_BASE}/lib
    -L${LCG_BASE}/lib64
    -lLHAPDF
    -lfmt

    # -L${CMAKE_SOURCE_DIR}/NanoMUSiC/MUSiC-Utils/lib
    -lz
    -lstdc++fs
    -L/cvmfs/sft.cern.ch/lcg/views/LCG_104/x86_64-el9-gcc12-opt/lib
    -L/cvmfs/sft.cern.ch/lcg/views/LCG_104/x86_64-el9-gcc12-opt/lib64
)

# add_executable(nano_music src/main.cpp /
# src/GeneratorFilters.cpp /
# src/EventAnalyzer/generator.cpp /
# src/EventAnalyzer/event_filters.cpp /
# src/EventAnalyzer/trigger.cpp /
# src/EventAnalyzer/scale_factors.cpp /
# src/EventAnalyzer/scale_factors_muons.cpp /
# src/JetCorrector.cpp /
# src/EventAnalyzer/scale_factors_egamma.cpp /
# src/EventAnalyzer/scale_factors_bjets.cpp /
# src/EventAnalyzer/transformations.cpp /
# src/EventAnalyzer/object_selection.cpp /
# src/EventAnalyzer/set_objects.cpp /
# src/EventAnalyzer/EventAnalyzer.cpp /
# src/TaskConfiguration.cpp src/NanoObjects.cpp /
# src/TOMLConfig.cpp src/CorrectionSets.cpp /
# external/roccor/RoccoR.cc /
# src/BinLimits.cpp)
# target_link_libraries(nano_music LINK_PUBLIC ROOT::RIO ROOT::Net)
# install(TARGETS nano_music DESTINATION ${CMAKE_SOURCE_DIR}/bin)

# # #####################################################
# # #####################################################
# # #####################################################
# add_link_options(
# -lboost_program_options
# )

# # nanoaod_content_printer
# add_executable(nanoaod_content_printer src/nanoaod_content_printer.cpp)
# target_link_libraries(nanoaod_content_printer LINK_PUBLIC NanoAODReader ROOT::RIO ROOT::Net)
# install(TARGETS nanoaod_content_printer DESTINATION ${CMAKE_SOURCE_DIR}/bin)
