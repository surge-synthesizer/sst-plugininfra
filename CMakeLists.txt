cmake_minimum_required(VERSION 3.10)
project(sst-plugininfra VERSION 0.5 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(SST_PLUGININFRA_BUILD_TESTS true)

option(SST_PLUGININFRA_FILESYSTEM_FORCE_PLATFORM "Use std::filesystem from the platform headers no matter what" OFF)
option(SST_PLUGININFRA_PROVIDE_TINYXML "Provide tinyxml as a target" ON)
option(SST_PLUGININFRA_PROVIDE_STRNATCMP "Provide strnatcmp as a target" ON)
option(SST_PLUGININFRA_PROVIDE_MINIZ "Provide libzip" ${PROJECT_IS_TOP_LEVEL})
option(SST_PLUGININFRA_PROVIDE_PATCHBASE "Provide patchbase" OFF)
option(SST_PLUGININFRA_BUILD_TESTS " Add targets for building and running sst-cpputils tests " ${PROJECT_IS_TOP_LEVEL})

# # Tests need this
if(${SST_PLUGININFRA_BUILD_TESTS})
    include(cmake/git-version-functions.cmake)
    version_from_git()
endif()

add_subdirectory(libs/filesystem)

if(${SST_PLUGININFRA_PROVIDE_TINYXML})
    add_subdirectory(libs/tinyxml)
    add_library(${PROJECT_NAME}::tinyxml ALIAS tinyxml)
endif()

if(${SST_PLUGININFRA_PROVIDE_STRNATCMP})
    add_subdirectory(libs/strnatcmp)
    add_library(${PROJECT_NAME}::strnatcmp ALIAS strnatcmp)
endif()

if(${SST_PLUGININFRA_PROVIDE_MINIZ})
    # This is basically the pain of windows not having a zlib
    # and libzip using find_package so this fakes out an install
    # in the build dir at configure time and then points at it
    set(BUILD_SHARED_LIBS OFF)
    add_subdirectory(libs/miniz)
    add_library(${PROJECT_NAME}::miniz ALIAS miniz)
endif()

if(${SST_PLUGININFRA_PROVIDE_PATCHBASE})
    if(NOT TARGET sst-basic-blocks)
        message(FATAL_ERROR "To provide PatchBase, PLUGININFRA needs basic blocks")
    endif()

    if(NOT TARGET tinyxml)
        message(FATAL_ERROR "To provide PatchBase, PLUGININFRA needs tinyxml")
    endif()

    if(NOT TARGET clap)
        message(FATAL_ERROR "To provide PatchBase, PLUGININFRA needs clap")
    endif()

    add_library(patchbase INTERFACE)
    target_include_directories(patchbase INTERFACE include)
    target_compile_definitions(patchbase INTERFACE SST_PLUGININFRA_PATCHBASE=1)
    target_link_libraries(patchbase INTERFACE tinyxml sst-basic-blocks clap)
    add_library(${PROJECT_NAME}::patchbase ALIAS patchbase)
endif()

add_library(version_information STATIC)

string(TIMESTAMP SSTPI_VERS_BUILD_DATE "%Y-%m-%d" UTC)
string(TIMESTAMP SSTPI_VERS_BUILD_YEAR "%Y" UTC)
string(TIMESTAMP SSTPI_VERS_BUILD_TIME "%H:%M:%S" UTC)

configure_file(src/version_information_in.cpp ${CMAKE_BINARY_DIR}/geninclude/version_information.cpp)
target_sources(version_information PRIVATE ${CMAKE_BINARY_DIR}/geninclude/version_information.cpp)
target_include_directories(version_information PUBLIC include)
target_compile_definitions(version_information PUBLIC SST_PLUGININFRA_HAS_VERSION_INFORMATION=1)
add_library(${PROJECT_NAME}::version_information ALIAS version_information)

configure_file(src/paths_subst.cpp.in ${CMAKE_BINARY_DIR}/gen/paths_subst.cpp)

add_library(${PROJECT_NAME} STATIC EXCLUDE_FROM_ALL ${CMAKE_BINARY_DIR}/gen/paths_subst.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} PUBLIC ${PROJECT_NAME}::filesystem ${PROJECT_NAME}::strnatcmp)

target_sources(${PROJECT_NAME} PRIVATE src/cpufeatures.cpp)

if(WIN32)
    target_sources(${PROJECT_NAME} PRIVATE src/paths_windows.cpp src/misc_windows.cpp)
    target_compile_definitions(${PROJECT_NAME} PRIVATE WINDOWS=1)
    target_compile_definitions(${PROJECT_NAME} PUBLIC SST_COMMAND_CTRL_SAME_KEY=1)
elseif(APPLE)
    target_sources(${PROJECT_NAME} PRIVATE src/paths_macos.mm src/misc_macos.mm)
    target_compile_definitions(${PROJECT_NAME} PRIVATE MAC=1)
    target_compile_definitions(${PROJECT_NAME} PUBLIC SST_COMMAND_CTRL_SAME_KEY=0)
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework Foundation")
else()
    target_sources(${PROJECT_NAME} PRIVATE src/paths_linux.cpp src/misc_linux.cpp)
    target_compile_definitions(${PROJECT_NAME} PRIVATE LINUX=1)
    target_compile_definitions(${PROJECT_NAME} PUBLIC SST_COMMAND_CTRL_SAME_KEY=1)
    target_link_libraries(${PROJECT_NAME} PUBLIC dl)
endif()

set_property(TARGET ${PROJECT_NAME} PROPERTY C_VISIBILITY_PRESET hidden)
set_property(TARGET ${PROJECT_NAME} PROPERTY VISIBILITY_INLINES_HIDDEN ON)

if(${SST_PLUGININFRA_BUILD_TESTS})
    add_subdirectory(tests)
endif()
