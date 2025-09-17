
function(version_from_git)
    find_package(Git)
    if (NOT Git_FOUND )
        message(FATAL_ERROR "Unable to find GIT")
    endif()

    # Make this an arg
    set(WD ".")

    execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
            WORKING_DIRECTORY ${WD}
            OUTPUT_VARIABLE GIT_BRANCH
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
            WORKING_DIRECTORY ${WD}
            OUTPUT_VARIABLE GIT_COMMIT_HASH
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    execute_process(
            COMMAND ${GIT_EXECUTABLE} describe --tags --exact-match
            WORKING_DIRECTORY ${WD}
            OUTPUT_VARIABLE GIT_TAG
            ERROR_VARIABLE GIT_TAG_ERROR
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if ("${GIT_TAG}" STREQUAL "")
        set(GIT_TAG "-no-tag-")
    endif()

    set(GIT_BRANCH ${GIT_BRANCH} PARENT_SCOPE)
    set(GIT_COMMIT_HASH ${GIT_COMMIT_HASH} PARENT_SCOPE)
    set(GIT_TAG ${GIT_TAG} PARENT_SCOPE)
endfunction(version_from_git)

function(version_from_versionfile)
    set(oneValueArgs VERSION_FILE VAR_PREFIX)
    cmake_parse_arguments(VF "" "${oneValueArgs}" "" ${ARGN} )
    if (NOT DEFINED VF_VERSION_FILE)
        message(FATAL_ERROR "version_from_versionfile requires a versionfile argument")
    endif()
    if (NOT DEFINED VF_VAR_PREFIX)
        set(VF_VAR_PREFIX "")
    else()
        set(VF_VAR_PREFIX "${VF_VAR_PREFIX}_")
    endif()

    file(STRINGS "${VF_VERSION_FILE}" VINF)
    list(LENGTH VINF vlen)
    if ("${vlen}" EQUAL 5)
        list(GET VINF 1 hash)
        list(GET VINF 2 tag)
        list(GET VINF 3 branch)
        list(GET VINF 4 iver)

        set(${VF_VAR_PREFIX}GIT_COMMIT_HASH "${hash}" PARENT_SCOPE)
        set(${VF_VAR_PREFIX}GIT_BRANCH "${branch}" PARENT_SCOPE)
        set(${VF_VAR_PREFIX}GIT_TAG "${tag}" PARENT_SCOPE)

        set(${VF_VAR_PREFIX}GIT_IMPLIED_DISPLAY_VERSION "${iver}" PARENT_SCOPE)
    else()
        message(WARINGO "VersionInfo file `${V1_VERSION_FILE}` not complete (${vlen} != 5)")
    endif()
endfunction(version_from_versionfile)

function(implied_display_version_from_git)
    set(oneValueArgs BRANCH TAG HASH NIGHTLY_NAME)
    cmake_parse_arguments(IV "" "${oneValueArgs}" "" ${ARGN} )

    if (NOT DEFINED IV_NIGHTLY_NAME)
        if (DEFINED GITHUB_ACTIONS_BUILD)
            set(IV_NIGHTLY_NAME "Nightly")
        else()
            set(IV_NIGHTLY_NAME "Local")
        endif()
    endif()

    set(res "unkown")
    if (DEFINED IV_HASH)
        set(res "git_${IV_HASH}")
    endif()

    if (DEFINED IV_BRANCH)
        if ("${IV_BRANCH}" STREQUAL "main")
            string(TIMESTAMP tsutc "%Y-%m-%d" UTC)
            set(res "${IV_NIGHTLY_NAME}-${tsutc}")
        elseif ("${IV_BRANCH}" STREQUAL "HEAD")
            string(TIMESTAMP tsutc "%Y-%m-%d" UTC)
            set(res "${IV_NIGHTLY_NAME}-${tsutc}")
        else()
            if (NOT DEFINED GITHUB_ACTIONS_BUILD)
                string(TIMESTAMP tsutc "%Y-%m-%d" UTC)
                set(res "${IV_BRANCH}-${tsutc}")
            endif()
        endif()
    endif()

    if (DEFINED IV_TAG)
        string(REGEX MATCH "^(v[0-9].*)$" tag ${IV_TAG})
        if (NOT "${tag}" STREQUAL "")
            set(res "${tag}")
        endif()
    endif()
    set(GIT_IMPLIED_DISPLAY_VERSION "${res}" PARENT_SCOPE)
endfunction()


function(generate_version_file)
    set(oneValueArgs FILE)
    cmake_parse_arguments(GVF "" "${oneValueArgs}" "" ${ARGN} )

    if (NOT DEFINED GVF_FILE)
        message(STATUS "GVF_FILE not specified; using default name BUILD_VERSION")
        set(GVF_FILE "BUILD_VERSION")
    endif()

    message(STATUS "Generating version info file '${GVF_FILE}'")
    version_from_git()
    implied_display_version_from_git(HASH ${GIT_COMMIT_HASH} TAG ${GIT_TAG} BRANCH ${GIT_BRANCH})
    message(STATUS "generated  : branch=${GIT_BRANCH} hash=${GIT_COMMIT_HASH} tag=${GIT_TAG} vers=${GIT_IMPLIED_DISPLAY_VERSION}")

    file(WRITE "${GVF_FILE}" "SST_VERSION_INFO\n")
    file(APPEND "${GVF_FILE}" "${GIT_COMMIT_HASH}\n")
    file(APPEND "${GVF_FILE}" "${GIT_TAG}\n")
    file(APPEND "${GVF_FILE}" "${GIT_BRANCH}\n")
    file(APPEND "${GVF_FILE}" "${GIT_IMPLIED_DISPLAY_VERSION}\n")

    # test the result
    version_from_versionfile(VERSION_FILE "${GVF_FILE}" VAR_PREFIX TST)
    message(STATUS "restored   : branch=${TST_GIT_BRANCH} hash=${TST_GIT_COMMIT_HASH} tag=${TST_GIT_TAG} vers=${TST_GIT_IMPLIED_DISPLAY_VERSION}")
endfunction(generate_version_file)

function(version_from_versionfile_or_git)
    set(oneValueArgs FILE NIGHTLY_NAME)
    cmake_parse_arguments(GVF "" "${oneValueArgs}" "" ${ARGN} )

    if (NOT DEFINED GVF_FILE)
        set(GVF_FILE "${CMAKE_SOURCE_DIR}/BUILD_VERSION")
    endif()

    if (NOT DEFINED GVF_NIGHTLY_NAME)
        if (DEFINED GITHUB_ACTIONS_BUILD)
            set(GVF_NIGHTLY_NAME "Nightly")
        else()
            set(GVF_NIGHTLY_NAME "Local")
        endif()
    endif()

    message(STATUS "Looking for file ${GVF_FILE}")
    if(EXISTS "${GVF_FILE}")
        message(STATUS "File Exists")
        version_from_versionfile(VERSION_FILE "${GVF_FILE}")
    else()
        message(STATUS "Determining version from git")
        version_from_git()
        implied_display_version_from_git(HASH ${GIT_COMMIT_HASH} TAG ${GIT_TAG} BRANCH ${GIT_BRANCH} NIGHTLY_NAME ${GVF_NIGHTLY_NAME})
    endif()



    string( TIMESTAMP L_SSTPI_VERS_BUILD_DATE "%Y-%m-%d" UTC)
    string( TIMESTAMP L_SSTPI_VERS_BUILD_YEAR "%Y" UTC)
    string( TIMESTAMP L_SSTPI_VERS_BUILD_TIME "%H:%M:%S" UTC)

    set(SSTPI_VERS_BUILD_DATE ${L_SSTPI_VERS_BUILD_DATE} PARENT_SCOPE)
    set(SSTPI_VERS_BUILD_YEAR ${L_SSTPI_VERS_BUILD_YEAR} PARENT_SCOPE)
    set(SSTPI_VERS_BUILD_TIME ${L_SSTPI_VERS_BUILD_TIME} PARENT_SCOPE)

    cmake_host_system_information(RESULT L_FQDN QUERY FQDN)
    set(SSTPI_FQDN ${L_FQDN} PARENT_SCOPE)

    set(GIT_BRANCH ${GIT_BRANCH} PARENT_SCOPE)
    set(GIT_COMMIT_HASH ${GIT_COMMIT_HASH} PARENT_SCOPE)
    set(GIT_TAG ${GIT_TAG} PARENT_SCOPE)
    set(GIT_IMPLIED_DISPLAY_VERSION ${GIT_IMPLIED_DISPLAY_VERSION} PARENT_SCOPE)
endfunction(version_from_versionfile_or_git)


if (DEFINED GENERATE_BUILD_VERSION)
    generate_version_file()
endif()