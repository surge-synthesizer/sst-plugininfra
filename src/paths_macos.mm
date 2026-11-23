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

namespace
{
fs::path userDirectory(NSSearchPathDirectory which)
{
    auto *u = [[NSFileManager defaultManager] URLsForDirectory:which inDomains:NSUserDomainMask]
                  .firstObject;
    if (!u)
        return fs::path{};
    return fs::path{[u fileSystemRepresentation]};
}
} // namespace

fs::path bestDocumentsVendorFolderPathFor(const std::string &vendorName,
                                          const std::string &productName)
{
    auto sub = vendorName.empty() ? fs::path{productName} : fs::path{vendorName} / productName;
    auto music = userDirectory(NSMusicDirectory);
    auto docs = userDirectory(NSDocumentDirectory);

    // apple wants ~/Music but an existing ~/Documents folder stays put
    std::error_code ec;
    if (!music.empty() && fs::is_directory(music / sub, ec))
        return music / sub;
    if (!docs.empty() && fs::is_directory(docs / sub, ec))
        return docs / sub;
    if (!music.empty())
        return music / sub;
    if (!docs.empty())
        return docs / sub;
    return fs::path{};
}

fs::path bestLibrarySharedVendorFolderPathFor(const std::string &vendorName,
                                              const std::string &productName, bool userLevel)
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
