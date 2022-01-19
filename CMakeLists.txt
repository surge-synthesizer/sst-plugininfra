cmake_minimum_required(VERSION 3.10)
project(sst-plugininfra VERSION 0.5 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE include)

add_subdirectory(libs/filesystem)
add_subdirectory(libs/tinyxml)
add_library(${PROJECT_NAME}::tinyxml ALIAS tinyxml)


get_directory_property(parent_dir PARENT_DIRECTORY)
if ("${parent_dir}" STREQUAL "")
    set(is_toplevel 1)
else ()
    set(is_toplevel 0)
endif ()
option(SST_PLUGININFRA_BUILD_TESTS "Add targets for building and running sst-cpputils tests" ${is_toplevel})

if (SST_PLUGININFRA_BUILD_TESTS)
    add_subdirectory(tests)
endif ()
