cmake_minimum_required(VERSION 3.10)
project(sst-plugininfra VERSION 0.5 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)


option(SST_PLUGININFRA_FILESYSTEM_FORCE_PLATFORM "Use std::filesystem from the platform headers no matter what" OFF)
option(SST_PLUGININFRA_PROVIDE_TINYXML "Provide tinyxml as a target" ON)
option(SST_PLUGININFRA_PROVIDE_STRNATCMP "Provide strnatcmp as a target" ON)
option(SST_PLUGININFRA_PROVIDE_LIBZIP "Provide libzip" OFF)

add_subdirectory(libs/filesystem)
if (${SST_PLUGININFRA_PROVIDE_TINYXML})
    add_subdirectory(libs/tinyxml)
    add_library(${PROJECT_NAME}::tinyxml ALIAS tinyxml)
endif ()

if (${SST_PLUGININFRA_PROVIDE_STRNATCMP})
    add_subdirectory(libs/strnatcmp)
    add_library(${PROJECT_NAME}::strnatcmp ALIAS strnatcmp)
endif ()

if (${SST_PLUGININFRA_PROVIDE_LIBZIP})
    set(BUILD_SHARED_LIBS OFF)

    if (WIN32)
        set(ZLIB_BUILD ${CMAKE_BINARY_DIR}/zlibcustom)
        message(STATUS "Executing ZLib Build Process")
        execute_process(WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
                COMMAND ${CMAKE_COMMAND} -B "${ZLIB_BUILD}" -DBUILD_SHARED_LIBS=OFF -S libs/zlib
                RESULT_VARIABLE R
                OUTPUT_VARIABLE O
                ERROR_VARIABLE E)
        if (${R})
            message(FATAL_ERROR "Could not configure zlib - ${E}")
        endif ()
        execute_process(WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
                COMMAND ${CMAKE_COMMAND} --build "${ZLIB_BUILD}"
                RESULT_VARIABLE R
                OUTPUT_VARIABLE O
                ERROR_VARIABLE E)

        execute_process(WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
                COMMAND ${CMAKE_COMMAND} -E copy "libs/zlib/*.h" "${ZLIB_BUILD}"
                RESULT_VARIABLE R
                OUTPUT_VARIABLE O
                ERROR_VARIABLE E)
        if (${R})
            message(FATAL_ERROR "Could not copy headers - ${R} ${E}")
        endif ()
        set(ZLIB_INCLUDE_DIR ${ZLIB_BUILD})
        set(ZLIB_LIBRARY ${ZLIB_BUILD})
    endif ()
    add_subdirectory(libs/libzip)
    if (WIN32)
        target_include_directories(zip PUBLIC
                ${ZLIB_BUILD}
                ${CMAKE_CURRENT_LIST_DIR}/libs/zlib
        )
    endif ()
    add_library(${PROJECT_NAME}::libzip ALIAS zip)
endif ()
configure_file(src/paths_subst.cpp.in ${CMAKE_BINARY_DIR}/gen/paths_subst.cpp)

add_library(${PROJECT_NAME} STATIC EXCLUDE_FROM_ALL ${CMAKE_BINARY_DIR}/gen/paths_subst.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} PUBLIC ${PROJECT_NAME}::filesystem ${PROJECT_NAME}::strnatcmp)

target_sources(${PROJECT_NAME} PRIVATE src/cpufeatures.cpp)

if (WIN32)
    target_sources(${PROJECT_NAME} PRIVATE src/paths_windows.cpp src/misc_windows.cpp)
    target_compile_definitions(${PROJECT_NAME} PRIVATE WINDOWS=1)
    target_compile_definitions(${PROJECT_NAME} PUBLIC SST_COMMAND_CTRL_SAME_KEY=1)
elseif (APPLE)
    target_sources(${PROJECT_NAME} PRIVATE src/paths_macos.mm src/misc_macos.mm)
    target_compile_definitions(${PROJECT_NAME} PRIVATE MAC=1)
    target_compile_definitions(${PROJECT_NAME} PUBLIC SST_COMMAND_CTRL_SAME_KEY=0)
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework Foundation")
else ()
    target_sources(${PROJECT_NAME} PRIVATE src/paths_linux.cpp src/misc_linux.cpp)
    target_compile_definitions(${PROJECT_NAME} PRIVATE LINUX=1)
    target_compile_definitions(${PROJECT_NAME} PUBLIC SST_COMMAND_CTRL_SAME_KEY=1)
    target_link_libraries(${PROJECT_NAME} PUBLIC dl)
endif ()

set_property(TARGET ${PROJECT_NAME} PROPERTY C_VISIBILITY_PRESET hidden)
set_property(TARGET ${PROJECT_NAME} PROPERTY VISIBILITY_INLINES_HIDDEN ON)


get_directory_property(parent_dir PARENT_DIRECTORY)
if (" ${parent_dir} " STREQUAL " ")
    set(is_toplevel 1)
else ()
    set(is_toplevel 0)
endif ()
option(SST_PLUGININFRA_BUILD_TESTS " Add targets for building and running sst-cpputils tests " ${is_toplevel})

if (SST_PLUGININFRA_BUILD_TESTS)
    add_subdirectory(tests)
endif ()
