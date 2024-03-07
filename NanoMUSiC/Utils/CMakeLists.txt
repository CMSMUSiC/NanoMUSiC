add_custom_target(example ALL
    COMMAND
    cd ${CMAKE_SOURCE_DIR}/NanoMUSiC/Utils/example && go install
    VERBATIM
)

add_custom_target(nanoaod_skimmer ALL
    COMMAND
    cd ${CMAKE_SOURCE_DIR}/NanoMUSiC/Utils/nanoaod_skimmer && go install
    VERBATIM
)

add_custom_target(gfal-ls-recursive ALL
    COMMAND
    cd ${CMAKE_SOURCE_DIR}/NanoMUSiC/Utils/gfal-ls-recursive && go install
    VERBATIM
)
