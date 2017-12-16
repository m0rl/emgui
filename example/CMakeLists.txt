add_custom_target(example COMMENT "Build emgui example")

add_executable(index EXCLUDE_FROM_ALL main.cpp)

get_filename_component(EXAMPLE_SHELL_FILE_TEMPLATE "static/index.html" ABSOLUTE)
set(EXAMPLE_LINK_FLAGS
    "-s USE_SDL=2 -s FETCH=1 --shell-file ${EXAMPLE_SHELL_FILE_TEMPLATE}")
set_target_properties(index PROPERTIES LINK_FLAGS ${EXAMPLE_LINK_FLAGS})
set_target_properties(index PROPERTIES OUTPUT_NAME "index" SUFFIX ".html")

target_compile_options(index PRIVATE -Wall -pedantic -Werror)
target_link_libraries(index emgui)

add_dependencies(example index)
set_target_properties(example PROPERTIES EXCLUDE_FROM_ALL TRUE)
