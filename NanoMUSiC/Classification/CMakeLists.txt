execute_process(COMMAND root-config --cflags OUTPUT_VARIABLE ROOT_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
separate_arguments(ROOT_CFLAGS)
add_compile_options(-g)

add_compile_options(-I/cvmfs/sft.cern.ch/lcg/views/LCG_106/x86_64-el9-gcc13-opt/include)

# add_compile_options(${ROOT_CFLAGS} ${CORRECTIONLIB_FLAGS} -fdiagnostics-color=always -MD -MP -O3 -Wall -march=native -pthread -m64 -ffloat-store)
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


add_library(EventClass SHARED
    src/EventClass.cpp //
    ${CMAKE_SOURCE_DIR}/NanoMUSiC/MUSiC/src/BinLimits.cpp //
)
target_include_directories(EventClass PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(EventClass LINK_PUBLIC ROOT::RIO ROOT::Net)
install(TARGETS EventClass DESTINATION ${CMAKE_SOURCE_DIR}/lib)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libEventClass_rdict.pcm DESTINATION ${CMAKE_SOURCE_DIR}/lib)

add_library(Distribution SHARED
	src/Distribution.cpp //
)
target_include_directories(Distribution PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(Distribution LINK_PUBLIC ROOT::RIO ROOT::Net)
install(TARGETS Distribution DESTINATION ${CMAKE_SOURCE_DIR}/lib)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libDistribution_rdict.pcm DESTINATION ${CMAKE_SOURCE_DIR}/lib)


message(STATUS "Generating EventClass dictionaries ...")
ROOT_GENERATE_DICTIONARY(EventClassDict include/EventClass.hpp include/ValidationContainer.hpp include/ZToLepLepX.hpp MODULE EventClass LINKDEF include/EventClassLinkDef.h)
message(STATUS "Done.")

message(STATUS "Generating Distribution dictionaries ...")
ROOT_GENERATE_DICTIONARY(DistributionDict include/Distribution.hpp  MODULE Distribution LINKDEF include/DistributionLinkDef.h)
message(STATUS "Done.")

# python binds
find_package(pybind11 REQUIRED)
pybind11_add_module(classification_imp MODULE
    src/classification.cpp //
    ${CMAKE_SOURCE_DIR}/NanoMUSiC/MUSiC/src/BinLimits.cpp //
    ${CMAKE_SOURCE_DIR}/NanoMUSiC/MUSiC/src/JetCorrector.cpp //
    src/GeneratorFilters.cpp //
    src/NanoAODGenInfo.cpp //
    src/ZToLepLepX.cpp //
    src/GammaPlusJet.cpp //
    src/TTBarTo1Lep2Bjet2JetMET.cpp //
    src/WToLepNuX.cpp //
    src/BTagEffMap.cpp //
    # src/WToLepNu_eff.cpp //
    # src/ZToTauTauLepX.cpp //
    # src/Distribution.cpp //
    src/pybind_imp.cpp //
)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
target_include_directories(classification_imp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(classification_imp LINK_PUBLIC ROOT::RIO ROOT::Net correctionlib)
target_link_libraries(classification_imp LINK_PRIVATE EventClass Distribution)
install(TARGETS classification_imp DESTINATION ${CMAKE_SOURCE_DIR}/lib/python)




