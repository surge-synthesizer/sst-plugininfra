/*
 * sst-plugininfra - an open source library of plugin infrastructure
 * built by Surge Synth Team.
 *
 * Copyright 2018-2024, various authors, as described in the GitHub
 * transaction log.
 *
 * sst-effects is released under the MIT License. It has subordinate
 * libraries with licenses as described in libs/
 *
 * All source in sst-plugininfra available at
 * https://github.com/surge-synthesizer/sst-plugininfra
 */

#include <sst/plugininfra/version_information.h>

static_assert("@GIT_COMMIT_HASH@"[0] != '\0',
              "GIT_COMMIT_HASH not defined. Run cmake/git-version-functions.cmake commands to use "
              "this library");
namespace sst::plugininfra
{
// clang-format off
const char *VersionInformation::git_commit_hash{"@GIT_COMMIT_HASH@"};
const char *VersionInformation::git_branch{"@GIT_BRANCH@"};
const char *VersionInformation::git_tag{"@GIT_TAG@"};
const char *VersionInformation::git_implied_display_version{"@GIT_IMPLIED_DISPLAY_VERSION@"};

const char *VersionInformation::project_version_major{"@CMAKE_PROJECT_VERSION_MAJOR@"};
const char *VersionInformation::project_version_minor{"@CMAKE_PROJECT_VERSION_MINOR@"};
const char *VersionInformation::project_version_patch{"@CMAKE_PROJECT_VERSION_PATCH@"};
const char *VersionInformation::project_version_tweak{"@CMAKE_PROJECT_VERSION_TWEAK@"};
const char *VersionInformation::project_version{
    "@CMAKE_PROJECT_VERSION_MAJOR@.@CMAKE_PROJECT_VERSION_MINOR@.@CMAKE_PROJECT_VERSION_PATCH@"};
const char *VersionInformation::project_version_and_hash{
    "@CMAKE_PROJECT_VERSION_MAJOR@.@CMAKE_PROJECT_VERSION_MINOR@.@CMAKE_PROJECT_VERSION_PATCH@.@GIT_COMMIT_HASH@"};

const char *VersionInformation::project_version_full{
    "@CMAKE_PROJECT_VERSION_MAJOR@.@CMAKE_PROJECT_VERSION_MINOR@.@CMAKE_PROJECT_VERSION_PATCH@.@CMAKE_PROJECT_VERSION_TWEAK@"};

const char *VersionInformation::cmake_source_dir{"@CMAKE_SOURCE_DIR@"};
const char *VersionInformation::cmake_binary_dir{"@CMAKE_BINARY_DIR@"};
const char *VersionInformation::cmake_install_prefix{"@CMAKE_INSTALL_PREFIX@"};
const char *VersionInformation::cmake_compiler_id{"@CMAKE_CXX_COMPILER_ID@"};
const char *VersionInformation::cmake_compiler_version{"@CMAKE_CXX_COMPILER_VERSION@"};
const char *VersionInformation::cmake_compiler{
    "@CMAKE_CXX_COMPILER_ID@-@CMAKE_CXX_COMPILER_VERSION@"};
const char *VersionInformation::cmake_system_name{"@CMAKE_SYSTEM_NAME@"};

const char *VersionInformation::build_date{"@SSTPI_VERS_BUILD_DATE@"};
const char *VersionInformation::build_year{"@SSTPI_VERS_BUILD_YEAR@"};
const char *VersionInformation::build_time{"@SSTPI_VERS_BUILD_TIME@"};
// clang-format on
} // namespace sst::plugininfra
