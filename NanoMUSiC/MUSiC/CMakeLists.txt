add_library(MUSiCEvent SHARED
        MUSiCEvent.hh
)

target_include_directories(MUSiCEvent PUBLIC ./)
target_compile_options(MUSiCEvent PUBLIC
        -MD
        -MP
        -O3
        -Wall
        -fPIC
        -fsignaling-nans
        -funsafe-math-optimizations
        -fno-rounding-math
        -fno-signaling-nans
        -fcx-limited-range
        -fno-associative-math
        -DMYPXLANA=EventClassFactory/AnalysisComposer.hh
        -pthread
        -std=c++17
        -m64
        -ffloat-store
)

target_link_options(MUSiCEvent PRIVATE -lz -fno-associative-math)

message(STATUS "Generating MUSiCEvent dictionaries ...")
ROOT_GENERATE_DICTIONARY(MUSiCEventDict MUSiCEvent.hh MODULE MUSiCEvent LINKDEF MUSiCEventLinkDef.h)
message(STATUS "Done.")

install(TARGETS MUSiCEventDict DESTINATION ${CMAKE_SOURCE_DIR}/lib)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libMUSiCEvent_rdict.pcm DESTINATION ${CMAKE_SOURCE_DIR}/lib)
install(TARGETS MUSiCEvent DESTINATION ${CMAKE_SOURCE_DIR}/lib)


###############################################################################
###############################################################################
###############################################################################
###############################################################################
###############################################################################

# enable tests
enable_testing()

# music (classification)
add_executable(music music.cc)
target_link_libraries(music LINK_PUBLIC PxlAnalyzer TEventClass MUSiCEvent)
install(TARGETS music DESTINATION ${CMAKE_SOURCE_DIR}/bin)

# MC
add_test(NAME test_music_2016APV_MC COMMAND music --batch --run-config ${CMAKE_SOURCE_DIR}/test/MC_2016APV.toml)
add_test(NAME test_music_2016_MC COMMAND music --batch --run-config ${CMAKE_SOURCE_DIR}/test/MC_2016.toml)
add_test(NAME test_music_2017_MC COMMAND music --batch --run-config ${CMAKE_SOURCE_DIR}/test/MC_2017.toml)
add_test(NAME test_music_2018_MC COMMAND music --batch --run-config ${CMAKE_SOURCE_DIR}/test/MC_2018.toml)

# Data
add_test(NAME test_music_2016APV_Data COMMAND music --batch --run-config ${CMAKE_SOURCE_DIR}/test/Data_2016APV.toml)
add_test(NAME test_music_2016_Data COMMAND music --batch --run-config ${CMAKE_SOURCE_DIR}/test/Data_2016.toml)
add_test(NAME test_music_2017_Data COMMAND music --batch --run-config ${CMAKE_SOURCE_DIR}/test/Data_2017.toml)
add_test(NAME test_music_2018_Data COMMAND music --batch --run-config ${CMAKE_SOURCE_DIR}/test/Data_2018.toml)

# #####################################################
# #####################################################
# #####################################################

# nano_music (classification)
add_executable(nano_music NanoMUSiC.cc)
target_link_libraries(nano_music LINK_PUBLIC PxlAnalyzer TEventClass MUSiCEvent)
install(TARGETS nano_music DESTINATION ${CMAKE_SOURCE_DIR}/bin)

# MC
add_test(NAME test_nano_music_2016APV_MC COMMAND nano_music --batch --run-config ${CMAKE_SOURCE_DIR}/test/MC_2016APV.toml)
add_test(NAME test_nano_music_2016_MC COMMAND nano_music --batch --run-config ${CMAKE_SOURCE_DIR}/test/MC_2016.toml)
add_test(NAME test_nano_music_2017_MC COMMAND nano_music --batch --run-config ${CMAKE_SOURCE_DIR}/test/MC_2017.toml)
add_test(NAME test_nano_music_2018_MC COMMAND nano_music --batch --run-config ${CMAKE_SOURCE_DIR}/test/MC_2018.toml)

# Data
add_test(NAME test_nano_music_2016APV_Data COMMAND nano_music --batch --run-config ${CMAKE_SOURCE_DIR}/test/Data_2016APV.toml)
add_test(NAME test_nano_music_2016_Data COMMAND nano_music --batch --run-config ${CMAKE_SOURCE_DIR}/test/Data_2016.toml)
add_test(NAME test_nano_music_2017_Data COMMAND nano_music --batch --run-config ${CMAKE_SOURCE_DIR}/test/Data_2017.toml)
add_test(NAME test_nano_music_2018_Data COMMAND nano_music --batch --run-config ${CMAKE_SOURCE_DIR}/test/Data_2018.toml)

# #####################################################
# #####################################################
# #####################################################

# nanoaod_content_printer
add_executable(nanoaod_content_printer nanoaod_content_printer.cc)
target_link_libraries(nanoaod_content_printer LINK_PUBLIC PxlAnalyzer)
install(TARGETS nanoaod_content_printer DESTINATION ${CMAKE_SOURCE_DIR}/bin)

# ##########
# ## MC ####
# ##########

# 2016APV
add_test(NAME test_nanoaod_content_printer_2016APV_MC COMMAND nanoaod_content_printer --input root://cms-xrd-global.cern.ch///store/mc/RunIISummer20UL16NanoAODAPVv9/DYJetsToLL_M-50_TuneCP5_13TeV-amcatnloFXFX-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_preVFP_v11-v1/120000/22F67F56-D2BD-D844-A646-87976559C1D7.root --output ${CMAKE_SOURCE_DIR}/docs/nanoaod_content_MC_2016APV.txt)

# 2016
add_test(NAME test_nanoaod_content_printer_2016_MC COMMAND nanoaod_content_printer --input root://cms-xrd-global.cern.ch///store/mc/RunIISummer20UL16NanoAODv9/DYJetsToLL_M-50_TuneCP5_13TeV-amcatnloFXFX-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v17-v1/30000/0082C29D-E74C-024A-BE9B-97B29EE7A4A2.root --output ${CMAKE_SOURCE_DIR}/docs/nanoaod_content_MC_2016.txt)

# 2017
add_test(NAME test_nanoaod_content_printer_2017_MC COMMAND nanoaod_content_printer --input root://cms-xrd-global.cern.ch///store/mc/RunIISummer20UL17NanoAODv9/DYJetsToLL_M-50_TuneCP5_13TeV-amcatnloFXFX-pythia8/NANOAODSIM/106X_mc2017_realistic_v9-v2/100000/04426F83-BF53-0440-B2D1-F2DD9AB96EB8.root --output ${CMAKE_SOURCE_DIR}/docs/nanoaod_content_MC_2017.txt)

# 2018
add_test(NAME test_nanoaod_content_printer_2018_MC COMMAND nanoaod_content_printer --input root://cms-xrd-global.cern.ch///store/mc/RunIISummer20UL18NanoAODv9/DYJetsToLL_M-50_TuneCP5_13TeV-amcatnloFXFX-pythia8/NANOAODSIM/106X_upgrade2018_realistic_v16_L1v1-v2/100000/13D0AD97-6B32-CB4C-BA87-5E37BA4CF20E.root --output ${CMAKE_SOURCE_DIR}/docs/nanoaod_content_MC_2018.txt)

# ############
# ## Data ####
# ############

# 2016APV
add_test(NAME test_nanoaod_content_printer_2016APV_Data COMMAND nanoaod_content_printer --input root://cms-xrd-global.cern.ch///store/data/Run2016C/SingleMuon/NANOAOD/HIPM_UL2016_MiniAODv2_NanoAODv9-v2/40000/0D1698EF-F93D-D84F-8529-4706B02CCB04.root --output ${CMAKE_SOURCE_DIR}/docs/nanoaod_content_Data_2016APV.txt)

# 2016
add_test(NAME test_nanoaod_content_printer_2016_Data COMMAND nanoaod_content_printer --input root://cms-xrd-global.cern.ch///store/data/Run2016F/SingleMuon/NANOAOD/UL2016_MiniAODv2_NanoAODv9-v1/130000/7F5791FE-262B-EA45-BE19-04E4F21FEE1E.root --output ${CMAKE_SOURCE_DIR}/docs/nanoaod_content_Data_2016.txt)

# 2017
add_test(NAME test_nanoaod_content_printer_2017_Data COMMAND nanoaod_content_printer --input root://cms-xrd-global.cern.ch///store/data/Run2017C/SingleMuon/NANOAOD/UL2017_MiniAODv2_NanoAODv9-v1/120000/037CED2F-27B3-DB4A-9A9D-C588589EDCE7.root --output ${CMAKE_SOURCE_DIR}/docs/nanoaod_content_Data_2017.txt)

# 2018
add_test(NAME test_nanoaod_content_printer_2018_Data COMMAND nanoaod_content_printer --input root://cms-xrd-global.cern.ch///store/data/Run2018C/SingleMuon/NANOAOD/UL2018_MiniAODv2_NanoAODv9-v2/100000/2B07B4C0-852B-9B4F-83FA-CA6B047542D1.root --output ${CMAKE_SOURCE_DIR}/docs/nanoaod_content_Data_2018.txt)