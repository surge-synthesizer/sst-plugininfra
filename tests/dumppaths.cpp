#include "sst/plugininfra/paths.h"
#include <iostream>

int main(int argc, char **argv)
{
    using namespace sst::plugininfra::paths;
    std::cout << "Path Display for Platform\n";
    std::cout << "home    : " << homePath().u8string() << "\n";
    std::cout << "bin     : " << sharedLibraryBinaryPath().u8string() << "\n";
    std::cout << "docs    : " << bestDocumentsFolderPathFor( "test-sst-plugininfra").u8string() << "\n";
    std::cout << "lib     : " << bestLibrarySharedFolderPathFor( "test-sst-plugininfra" ).u8string() << "\n";
    std::cout << "lib usr : " << bestLibrarySharedFolderPathFor( "test-sst-plugininfra", true ).u8string() << "\n";
}
