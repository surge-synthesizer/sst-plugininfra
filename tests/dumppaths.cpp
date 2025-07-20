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
    std::cout << "home    : " << homePath().u8string() << "\n";
    std::cout << "bin     : " << sharedLibraryBinaryPath().u8string() << "\n";
    std::cout << "docs    : " << bestDocumentsFolderPathFor("test-sst-plugininfra").u8string()
              << "\n";
    std::cout << "lib     : " << bestLibrarySharedFolderPathFor("test-sst-plugininfra").u8string()
              << "\n";
    std::cout << "lib usr : "
              << bestLibrarySharedFolderPathFor("test-sst-plugininfra", true).u8string() << "\n";
}
