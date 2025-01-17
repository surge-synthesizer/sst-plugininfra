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
#include <cstring>
#include <iostream>

int main(int argc, char **argv)
{
    std::cout << sst::plugininfra::VersionInformation::git_commit_hash << std::endl;
    if (strlen(sst::plugininfra::VersionInformation::git_commit_hash) == 0)
        return 1;

    return 0;
}