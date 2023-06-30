# enable tests
enable_testing()

add_subdirectory(MUSiC)
add_subdirectory(MUSiC-LightSkimmer)
add_subdirectory(MUSiC-Classification)

# add_subdirectory(MUSiC-Validation)
add_subdirectory(MUSiC-HeavyValidation)

add_subdirectory(TEventClass)
add_subdirectory(NanoEventClass)

# add_subdirectory(PxlAnalyzer)

# add_subdirectory(MUSiC-Utils)
# add_subdirectory(MUSiC-RoIScanner)
