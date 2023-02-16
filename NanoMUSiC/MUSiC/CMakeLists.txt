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
add_compile_options(-I/cvmfs/sft.cern.ch/lcg/views/LCG_102b/x86_64-centos7-gcc12-opt/include)
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
)

add_executable(nano_music src/main.cpp /
    src/GeneratorFilters.cpp /
    src/EventAnalyzer/generator.cpp /
    src/EventAnalyzer/event_filters.cpp /
    src/EventAnalyzer/trigger.cpp /
    src/EventAnalyzer/scale_factors.cpp /
    src/EventAnalyzer/scale_factors_muons.cpp /
    src/JetCorrector.cpp /
    src/EventAnalyzer/scale_factors_egamma.cpp /
    src/EventAnalyzer/scale_factors_bjets.cpp /
    src/EventAnalyzer/transformations.cpp /
    src/EventAnalyzer/object_selection.cpp /
    src/EventAnalyzer/set_objects.cpp /
    src/EventAnalyzer/EventAnalyzer.cpp /
    src/TaskConfiguration.cpp src/NanoObjects.cpp /
    src/TOMLConfig.cpp src/CorrectionSets.cpp /
    external/roccor/RoccoR.cc)
target_link_libraries(nano_music LINK_PUBLIC)
install(TARGETS nano_music DESTINATION ${CMAKE_SOURCE_DIR}/bin)

# MC
add_test(NAME test_nano_music_2016APV_MC COMMAND nano_music --batch --run-config ${CMAKE_SOURCE_DIR}/configs/task_configs/MC_Run2016APV.toml)
add_test(NAME test_nano_music_2016_MC COMMAND nano_music --batch --run-config ${CMAKE_SOURCE_DIR}/configs/task_configs/MC_Run2016.toml)
add_test(NAME test_nano_music_2017_MC COMMAND nano_music --batch --run-config ${CMAKE_SOURCE_DIR}/configs/task_configs/MC_Run2017.toml)
add_test(NAME test_nano_music_2018_MC COMMAND nano_music --batch --run-config ${CMAKE_SOURCE_DIR}/configs/task_configs/MC_Run2018.toml)

# Data
add_test(NAME test_nano_music_2016APV_Data COMMAND nano_music --batch --run-config ${CMAKE_SOURCE_DIR}/configs/task_configs/Data_Run2016APV.toml)
add_test(NAME test_nano_music_2016_Data COMMAND nano_music --batch --run-config ${CMAKE_SOURCE_DIR}/configs/task_configs/Data_Run2016.toml)
add_test(NAME test_nano_music_2017_Data COMMAND nano_music --batch --run-config ${CMAKE_SOURCE_DIR}/configs/task_configs/Data_Run2017.toml)
add_test(NAME test_nano_music_2018_Data COMMAND nano_music --batch --run-config ${CMAKE_SOURCE_DIR}/configs/task_configs/Data_Run2018.toml)

# #####################################################
# #####################################################
# #####################################################
add_link_options(
    -lboost_program_options
)

# nanoaod_content_printer
add_executable(nanoaod_content_printer src/nanoaod_content_printer.cpp)
target_link_libraries(nanoaod_content_printer LINK_PUBLIC NanoAODReader)
install(TARGETS nanoaod_content_printer DESTINATION ${CMAKE_SOURCE_DIR}/bin)

# #########
# # MC ####
# #########

# 2016APV
# add_test(NAME test_nanoaod_content_printer_2016APV_MC COMMAND nanoaod_content_printer --input root://cms-xrd-global.cern.ch///store/mc/RunIISummer20UL16NanoAODAPVv9/DYJetsToLL_M-50_TuneCP5_13TeV-amcatnloFXFX-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_preVFP_v11-v1/120000/22F67F56-D2BD-D844-A646-87976559C1D7.root --output ${CMAKE_SOURCE_DIR}/docs/nanoaod_content_MC_2016APV.txt)

# 2016
# add_test(NAME test_nanoaod_content_printer_2016_MC COMMAND nanoaod_content_printer --input root://cms-xrd-global.cern.ch///store/mc/RunIISummer20UL16NanoAODv9/DYJetsToLL_M-50_TuneCP5_13TeV-amcatnloFXFX-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v17-v1/30000/0082C29D-E74C-024A-BE9B-97B29EE7A4A2.root --output ${CMAKE_SOURCE_DIR}/docs/nanoaod_content_MC_2016.txt)

# 2017
# add_test(NAME test_nanoaod_content_printer_2017_MC COMMAND nanoaod_content_printer --input root://cms-xrd-global.cern.ch///store/mc/RunIISummer20UL17NanoAODv9/DYJetsToLL_M-50_TuneCP5_13TeV-amcatnloFXFX-pythia8/NANOAODSIM/106X_mc2017_realistic_v9-v2/100000/04426F83-BF53-0440-B2D1-F2DD9AB96EB8.root --output ${CMAKE_SOURCE_DIR}/docs/nanoaod_content_MC_2017.txt)

# 2018
# add_test(NAME test_nanoaod_content_printer_2018_MC COMMAND nanoaod_content_printer --input root://cms-xrd-global.cern.ch///store/mc/RunIISummer20UL18NanoAODv9/DYJetsToLL_M-50_TuneCP5_13TeV-amcatnloFXFX-pythia8/NANOAODSIM/106X_upgrade2018_realistic_v16_L1v1-v2/100000/13D0AD97-6B32-CB4C-BA87-5E37BA4CF20E.root --output ${CMAKE_SOURCE_DIR}/docs/nanoaod_content_MC_2018.txt)

# ###########
# # Data ####
# ###########

# 2016APV
# add_test(NAME test_nanoaod_content_printer_2016APV_Data COMMAND nanoaod_content_printer --input root://cms-xrd-global.cern.ch///store/data/Run2016C/SingleMuon/NANOAOD/HIPM_UL2016_MiniAODv2_NanoAODv9-v2/40000/0D1698EF-F93D-D84F-8529-4706B02CCB04.root --output ${CMAKE_SOURCE_DIR}/docs/nanoaod_content_Data_2016APV.txt)

# 2016
# add_test(NAME test_nanoaod_content_printer_2016_Data COMMAND nanoaod_content_printer --input root://cms-xrd-global.cern.ch///store/data/Run2016F/SingleMuon/NANOAOD/UL2016_MiniAODv2_NanoAODv9-v1/130000/7F5791FE-262B-EA45-BE19-04E4F21FEE1E.root --output ${CMAKE_SOURCE_DIR}/docs/nanoaod_content_Data_2016.txt)

# 2017
# add_test(NAME test_nanoaod_content_printer_2017_Data COMMAND nanoaod_content_printer --input root://cms-xrd-global.cern.ch///store/data/Run2017C/SingleMuon/NANOAOD/UL2017_MiniAODv2_NanoAODv9-v1/120000/037CED2F-27B3-DB4A-9A9D-C588589EDCE7.root --output ${CMAKE_SOURCE_DIR}/docs/nanoaod_content_Data_2017.txt)

# 2018
# add_test(NAME test_nanoaod_content_printer_2018_Data COMMAND nanoaod_content_printer --input root://cms-xrd-global.cern.ch///store/data/Run2018C/SingleMuon/NANOAOD/UL2018_MiniAODv2_NanoAODv9-v2/100000/2B07B4C0-852B-9B4F-83FA-CA6B047542D1.root --output ${CMAKE_SOURCE_DIR}/docs/nanoaod_content_Data_2018.txt)