project(filesystem VERSION 0.0.0 LANGUAGES CXX)

set(INC_DIR "${CMAKE_CURRENT_BINARY_DIR}/include")
set(HEADER_DIR "${INC_DIR}/${PROJECT_NAME}")

function(generate_header FS_HEADER FS_NAMESPACE FS_DEFINE)
    message(STATUS "${PROJECT_NAME}: Using ${FS_NAMESPACE} from ${FS_HEADER} with ${FS_DEFINE}")
    set(HEADER_FILE "${HEADER_DIR}/import.h")
    configure_file(src/import.h.in "${HEADER_FILE}" @ONLY)
    target_sources(${PROJECT_NAME} INTERFACE "${HEADER_FILE}")
    message(STATUS "Creating fs import via '${HEADER_FILE}'")
endfunction()

function(use_ghc_fs)
    # Don't export GHC symbols from ELF shared objects
    set(HEADER_FILE "${HEADER_DIR}/ghc-filesystem.h")
    file(STRINGS ghc-filesystem/include/ghc/filesystem.hpp HEADER_DATA NEWLINE_CONSUME NO_HEX_CONVERSION)
    string(REPLACE "__attribute__((visibility(\"default\")))" "" HEADER_DATA "${HEADER_DATA}")
    file(WRITE "${HEADER_FILE}" ${HEADER_DATA})

    add_library(${PROJECT_NAME} STATIC src/ghc-filesystem.cpp "${HEADER_FILE}")
    generate_header("\"filesystem/ghc-filesystem.h\"" "ghc::filesystem" "SST_PLUGINFRA_GHC_FS")
    target_compile_definitions(${PROJECT_NAME}
            INTERFACE GHC_FILESYSTEM_FWD
            PUBLIC GHC_RAISE_UNICODE_ERRORS GHC_WIN_WSTRING_STRING_TYPE
            PRIVATE GHC_FILESYSTEM_IMPLEMENTATION
            )
    target_include_directories(${PROJECT_NAME} PUBLIC "${INC_DIR}")
    set_property(TARGET ${PROJECT_NAME} PROPERTY C_VISIBILITY_PRESET hidden)
    set_property(TARGET ${PROJECT_NAME} PROPERTY VISIBILITY_INLINES_HIDDEN ON)
endfunction()

function(use_platform_fs)
    add_library(${PROJECT_NAME} INTERFACE)
    generate_header("<filesystem>" "std::filesystem" "SST_PLUGINFRA_PLATFORM_FS")
    target_include_directories(${PROJECT_NAME} INTERFACE "${INC_DIR}")
endfunction()

if (UNIX AND NOT APPLE)
    # with gcc11 on arch and latest ardour we get a crash in std::fs if we use the
    # os provided one so keep this as an option but default to off
    set(SST_PLUGININFRA_FILESYSTEM_FORCE_GHC ON CACHE BOOL "Force GHC filesystem implementation")
else ()
    set(SST_PLUGININFRA_FILESYSTEM_FORCE_GHC OFF CACHE BOOL "Force GHC filesystem implementation")
endif ()

if (${SST_PLUGININFRA_FILESYSTEM_FORCE_PLATFORM})
    message(STATUS "Forcing filesystem to platform")
    use_platform_fs()
elseif (${SST_PLUGININFRA_FILESYSTEM_FORCE_GHC})
    message(STATUS "Forcing filesystem to GHC")
    use_ghc_fs()
else ()
    message(STATUS "Detecting Filesystem Support")
    include(CheckCXXSourceCompiles)
    check_cxx_source_compiles("
    #include <filesystem>
    namespace fs = std::filesystem;
    int main(int, char **argv) {
      return fs::recursive_directory_iterator{fs::path{*argv}}.depth();
    }" SST_PLUGININFRA_FILESYSTEM_STD_FS_FOUND)
    if (SST_PLUGININFRA_FILESYSTEM_STD_FS_FOUND)
        use_platform_fs()
    else ()
        use_ghc_fs()
    endif ()
endif ()

if(MSVC)
    target_compile_options(${PROJECT_NAME} INTERFACE $<$<COMPILE_LANGUAGE:CXX>:/Zc:__cplusplus>)
else()
    target_compile_options(${PROJECT_NAME} INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-fno-char8_t>)
endif()

add_library(sst-plugininfra::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
