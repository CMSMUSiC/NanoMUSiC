# enable tests
enable_testing()

# music (classification)
add_executable(music music.cc)
target_link_libraries(music LINK_PUBLIC PxlAnalyzer TEventClass)
install(TARGETS music DESTINATION ${CMAKE_SOURCE_DIR}/bin)
add_test(NAME test_music_2017 COMMAND music --config ../test/MC.toml)

# nanoaod_content_printer
add_executable(nanoaod_content_printer nanoaod_content_printer.cc)
target_link_libraries(nanoaod_content_printer LINK_PUBLIC PxlAnalyzer)
install(TARGETS nanoaod_content_printer DESTINATION ${CMAKE_SOURCE_DIR}/bin)

# add_test(NAME test_nanoaod_content_printer_2017 COMMAND ./nanoaod_content_printer --config ../test/MC.toml)

# add_custom_target(validation music --config test/MC.toml)
