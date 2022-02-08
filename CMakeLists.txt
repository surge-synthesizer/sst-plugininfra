cmake_minimum_required(VERSION 3.10)
project(sst-plugininfra VERSION 0.5 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)


add_subdirectory(libs/filesystem)
add_subdirectory(libs/tinyxml)
add_library(${PROJECT_NAME}::tinyxml ALIAS tinyxml)
add_subdirectory(libs/strnatcmp)
add_library(${PROJECT_NAME}::strnatcmp ALIAS strnatcmp)


configure_file(src/paths_subst.cpp.in ${CMAKE_BINARY_DIR}/gen/paths_subst.cpp)

add_library(${PROJECT_NAME} EXCLUDE_FROM_ALL ${CMAKE_BINARY_DIR}/gen/paths_subst.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} PUBLIC ${PROJECT_NAME}::filesystem ${PROJECT_NAME}::strnatcmp)

target_sources(${PROJECT_NAME} PRIVATE src/cpufeatures.cpp)

if(WIN32)
  target_sources(${PROJECT_NAME} PRIVATE src/paths_windows.cpp)
  target_compile_definitions(${PROJECT_NAME} PRIVATE WINDOWS=1)
elseif(APPLE)
  target_sources(${PROJECT_NAME} PRIVATE src/paths_macos.mm)
  target_compile_definitions(${PROJECT_NAME} PRIVATE MAC=1)
  target_link_libraries(${PROJECT_NAME} PRIVATE "-framework Foundation")
else()
  target_sources(${PROJECT_NAME} PRIVATE src/paths_linux.cpp)
  target_compile_definitions(${PROJECT_NAME} PRIVATE LINUX=1)
  target_link_libraries(${PROJECT_NAME} PUBLIC dl)
endif()


get_directory_property(parent_dir PARENT_DIRECTORY)
if("${parent_dir}" STREQUAL "")
  set(is_toplevel 1)
else()
  set(is_toplevel 0)
endif()
option(SST_PLUGININFRA_BUILD_TESTS "Add targets for building and running sst-cpputils tests" ${is_toplevel})

if(SST_PLUGININFRA_BUILD_TESTS)
  add_subdirectory(tests)
endif()
