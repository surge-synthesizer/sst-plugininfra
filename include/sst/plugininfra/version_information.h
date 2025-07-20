/*
 * sst-plugininfra - an open source library of plugin infrastructure
 * built by Surge Synth Team.
 *
 * Copyright 2018-2025, various authors, as described in the GitHub
 * transaction log.
 *
 * sst-plugininfra is released under the MIT License. It has subordinate
 * libraries with licenses as described in libs/
 *
 * All source in sst-plugininfra available at
 * https://github.com/surge-synthesizer/sst-plugininfra
 */

#ifndef INCLUDE_SST_PLUGININFRA_VERSION_INFORMATION_H
#define INCLUDE_SST_PLUGININFRA_VERSION_INFORMATION_H

#ifdef SST_PLUGININFRA_HAS_VERSION_INFORMATION
namespace sst::plugininfra
{
struct VersionInformation
{
    static const char *git_commit_hash;
    static const char *git_branch;
    static const char *git_tag;
    static const char *git_implied_display_version;

    static const char *cmake_source_dir;
    static const char *cmake_binary_dir;
    static const char *cmake_install_prefix;
    static const char *cmake_compiler_id;
    static const char *cmake_compiler_version;
    static const char *cmake_compiler;
    static const char *cmake_system_name;

    static const char *project_version_major;
    static const char *project_version_minor;
    static const char *project_version_patch;
    static const char *project_version_tweak;
    static const char *project_version;          // major.minor.patch
    static const char *project_version_and_hash; // major.minor.patch.hash
    static const char *project_version_full;     // major.minor.patch.tweak

    // These are all UTC times
    static const char *build_date;
    static const char *build_year;
    static const char *build_time;
};
} // namespace sst::plugininfra
#endif
#endif // VERSION_SUPPORT_H
