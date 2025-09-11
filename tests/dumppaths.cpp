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
#include "sst/plugininfra/paths.h"
#include <iostream>

int main(int argc, char **argv)
{
    using namespace sst::plugininfra::paths;
    std::cout << "Path Display for Platform\n";
    std::cout << "home    : " << path_to_string(homePath()) << "\n";
    std::cout << "bin     : " << path_to_string(sharedLibraryBinaryPath()) << "\n";
    std::cout << "docs    : " << path_to_string(bestDocumentsFolderPathFor("test-sst-plugininfra"))
              << "\n";
    std::cout << "lib     : "
              << path_to_string(bestLibrarySharedFolderPathFor("test-sst-plugininfra")) << "\n";
    std::cout << "lib usr : "
              << path_to_string(bestLibrarySharedFolderPathFor("test-sst-plugininfra", true))
              << "\n";
}
