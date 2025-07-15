/*
 * paths_linux
 *
 * Semantics are:
 */

#include "filesystem/import.h"
#include "sst/plugininfra/paths.h"
#include <dlfcn.h>
#include <Foundation/Foundation.h>

namespace sst
{
namespace plugininfra
{
namespace paths
{
fs::path homePath()
{
    const char *const path = std::getenv("HOME");
    if (!path || !path[0])
        throw std::runtime_error{"The environment variable HOME is unset or empty"};
    return fs::path{path};
}

fs::path sharedLibraryBinaryPath()
{
    Dl_info info;
    if (!dladdr(reinterpret_cast<const void *>(&sharedLibraryBinaryPath), &info) ||
        !info.dli_fname[0])
    {
        // If dladdr(3) returns zero, dlerror(3) won't know why either
        throw std::runtime_error{"Failed to retrieve shared object file name"};
    }
    return fs::path{info.dli_fname};
}

fs::path bestDocumentsVendorFolderPathFor(const std::string &vendorName, const std::string &productName)
{
    auto *fileManager = [NSFileManager defaultManager];
    auto *resultURLs = [fileManager URLsForDirectory:NSDocumentDirectory
                                           inDomains:NSUserDomainMask];
    if (resultURLs)
    {
        auto *u = [resultURLs objectAtIndex:0];
        if (!vendorName.empty())
            return fs::path{[u fileSystemRepresentation]} / vendorName / productName;
        else
            return fs::path{[u fileSystemRepresentation]} / productName;
    }
    return fs::path{};
}

fs::path bestLibrarySharedVendorFolderPathFor(const std::string &vendorName, const std::string &productName, bool userLevel)
{
    auto *fileManager = [NSFileManager defaultManager];
    auto key = userLevel ? NSUserDomainMask : NSLocalDomainMask;
    auto *resultURLs = [fileManager URLsForDirectory:NSApplicationSupportDirectory inDomains:key];
    if (resultURLs)
    {
        auto *u = [resultURLs objectAtIndex:0];
        if (!vendorName.empty())
            return fs::path{[u fileSystemRepresentation]} / vendorName / productName;
        else
            return fs::path{[u fileSystemRepresentation]} / productName;
    }
    return fs::path{};
}
} // namespace paths
} // namespace plugininfra
} // namespace sst
