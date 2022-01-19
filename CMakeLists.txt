cmake_minimum_required(VERSION 3.10)
project(sst-plugininfras VERSION 0.5 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE include)

get_directory_property(parent_dir PARENT_DIRECTORY)
if ("${parent_dir}" STREQUAL "")
    set(is_toplevel 1)
else ()
    set(is_toplevel 0)
endif ()
option(SST_PLUGININFRA_BUILD_TESTS "Add targets for building and running sst-cpputils tests" ${is_toplevel})

if (SST_PLUGININFRA_BUILD_TESTS)
    add_executable(sst-plugininfras-tests)
    target_include_directories(sst-plugininfras-tests PRIVATE tests)
    target_link_libraries(sst-plugininfras-tests PRIVATE ${PROJECT_NAME})
    target_sources(sst-plugininfras-tests PRIVATE
            tests/tests.cpp)

    add_custom_command(TARGET sst-plugininfras-tests
            POST_BUILD
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMAND ${CMAKE_COMMAND} -E echo "Copying $<TARGET_FILE:sst-plugininfras-tests> to test-binary"
            COMMAND ${CMAKE_COMMAND} -E make_directory test-binary
            COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:sst-plugininfras-tests>" test-binary)
endif ()
