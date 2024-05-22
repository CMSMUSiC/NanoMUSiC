add_custom_target(gfal-ls-recursive ALL 
    COMMAND
    cd ${CMAKE_SOURCE_DIR}/NanoMUSiC/Utils/gfal-ls-recursive && go install
    VERBATIM
)
