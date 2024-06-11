# correctionlib compilation and linking flags
# execute_process(COMMAND correction config --cflags --ldflags --rpath OUTPUT_VARIABLE CORRECTIONLIB_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
# separate_arguments(CORRECTIONLIB_FLAGS)

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
    # ${CORRECTIONLIB_FLAGS}
    -lLHAPDF
    -lfmt
    -lz
    -lstdc++fs
    -L/cvmfs/sft.cern.ch/lcg/views/LCG_105a/x86_64-el9-gcc12-opt/lib
    -L/cvmfs/sft.cern.ch/lcg/views/LCG_105a/x86_64-el9-gcc12-opt/lib64
)

# python binds
find_package(pybind11 REQUIRED)
pybind11_add_module(classification_imp MODULE
    src/classification.cpp //
    ${CMAKE_SOURCE_DIR}/NanoMUSiC/MUSiC/src/BinLimits.cpp //
    ${CMAKE_SOURCE_DIR}/NanoMUSiC/MUSiC/src/JetCorrector.cpp //
    # ${CMAKE_SOURCE_DIR}/NanoMUSiC/NanoEventClass/src/NanoEventClass.cpp //
    src/GeneratorFilters.cpp //
    src/NanoAODGenInfo.cpp //
    src/ZToLepLepX.cpp //
    src/GammaPlusJet.cpp //
    src/TTBarTo1Lep2Bjet2JetMET.cpp //
    src/WToLepNuX.cpp //
    # src/WToLepNu_eff.cpp //
    # src/ZToTauTauLepX.cpp //
    src/EventClass.cpp //
    src/Distribution.cpp //
    src/pybind_imp.cpp //
)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
target_link_libraries(classification_imp LINK_PUBLIC ROOT::RIO ROOT::Net correctionlib)
target_include_directories(classification_imp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
install(TARGETS classification_imp DESTINATION ${CMAKE_SOURCE_DIR}/lib/python)


message(STATUS "Generating EventClass dictionaries ...")
# ROOT_GENERATE_DICTIONARY(EventClassDict include/EventClass.hpp include/ValidationContainer.hpp include/ZToLepLepX.hpp include/Distribution.hpp include/distribution_factory.hpp MODULE classification_imp LINKDEF include/EventClassLinkDef.h)
ROOT_GENERATE_DICTIONARY(EventClassDict include/EventClass.hpp include/ValidationContainer.hpp include/ZToLepLepX.hpp include/Distribution.hpp  MODULE classification_imp LINKDEF include/EventClassLinkDef.h)
message(STATUS "Done.")

install(TARGETS EventClassDict DESTINATION ${CMAKE_SOURCE_DIR}/lib)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libclassification_imp_rdict.pcm DESTINATION ${CMAKE_SOURCE_DIR}/lib/python)

