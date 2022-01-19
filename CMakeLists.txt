cmake_minimum_required(VERSION 3.10)
project(sst-plugininfra VERSION 0.5 LANGUAGES C CXX)

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
    add_executable(sst-plugininfra-tests)
    target_include_directories(sst-plugininfra-tests PRIVATE tests)
    target_link_libraries(sst-plugininfra-tests PRIVATE ${PROJECT_NAME})
    target_sources(sst-plugininfra-tests PRIVATE
            tests/tests.cpp)

    add_custom_command(TARGET sst-plugininfra-tests
            POST_BUILD
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMAND ${CMAKE_COMMAND} -E echo "Copying $<TARGET_FILE:sst-plugininfra-tests> to test-binary"
            COMMAND ${CMAKE_COMMAND} -E make_directory test-binary
            COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:sst-plugininfra-tests>" test-binary)
endif ()
