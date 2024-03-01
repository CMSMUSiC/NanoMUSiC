add_custom_target(example ALL
    COMMAND
    cd ${CMAKE_SOURCE_DIR}/NanoMUSiC/Utils/example && go install
    VERBATIM
)

