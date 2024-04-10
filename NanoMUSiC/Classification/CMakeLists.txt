# correctionlib compilation and linking flags
execute_process(COMMAND correction config --cflags --ldflags --rpath OUTPUT_VARIABLE CORRECTIONLIB_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
separate_arguments(CORRECTIONLIB_FLAGS)

# compilation options
execute_process(COMMAND root-config --cflags OUTPUT_VARIABLE ROOT_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
separate_arguments(ROOT_CFLAGS)
add_compile_options(-g)

add_compile_options(-I/cvmfs/sft.cern.ch/lcg/views/LCG_105a/x86_64-el9-gcc12-opt/include)

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
    -L/cvmfs/sft.cern.ch/lcg/views/LCG_105a/x86_64-el9-gcc12-opt/lib
    -L/cvmfs/sft.cern.ch/lcg/views/LCG_105a/x86_64-el9-gcc12-opt/lib64
)

add_library(Classification SHARED 
    src/classification.cpp //
    ${CMAKE_SOURCE_DIR}/NanoMUSiC/MUSiC/src/BinLimits.cpp //
    ${CMAKE_SOURCE_DIR}/NanoMUSiC/MUSiC/src/TOMLConfig.cpp //
    ${CMAKE_SOURCE_DIR}/NanoMUSiC/MUSiC/src/JetCorrector.cpp //
    ${CMAKE_SOURCE_DIR}/NanoMUSiC/NanoEventClass/src/NanoEventClass.cpp //
    src/GeneratorFilters.cpp //
    src/NanoAODGenInfo.cpp //
    src/ZToLepLepX.cpp //
    src/GammaPlusJet.cpp //
    src/TTBarTo1Lep2Bjet2JetMET.cpp //
    src/WToLepNuX.cpp //
    src/WToLepNu_eff.cpp //
    src/ZToTauTauLepX.cpp //
    src/EventClass.cpp //
)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
target_link_libraries(Classification LINK_PUBLIC ROOT::RIO ROOT::Net)
install(TARGETS Classification DESTINATION ${CMAKE_SOURCE_DIR}/lib)

add_executable(
    merger //
    src/merger.cpp
)
set_target_properties(merger PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(merger LINK_PUBLIC ROOT::RIO ROOT::Net)
install(TARGETS merger DESTINATION ${CMAKE_SOURCE_DIR}/bin)
