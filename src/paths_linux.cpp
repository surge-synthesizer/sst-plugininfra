/*
 * paths_linux
 *
 * Semantics are:
 */

#include "filesystem/import.h"
#include "sst/plugininfra/paths.h"
#include <stdexcept>
#include <stdlib.h>
#include <dlfcn.h>

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
    if (!dladdr(reinterpret_cast<const void *>(&sharedLibraryBinaryPath), &info) || !info.dli_fname[0])
    {
        // If dladdr(3) returns zero, dlerror(3) won't know why either
        throw std::runtime_error{"Failed to retrieve shared object file name"};
    }
    return fs::path{info.dli_fname};
}

/*
 * The waterfall we use here is as follows
 *
 * 1. Is XDG_DOCUMENTS_DIR set - if so use that with productName
 * 2. Does ~/Documents exists? If so use that
 * 3. Else use ~/.productName
 */
fs::path bestDocumentsFolderPathFor(const std::string &productName)
{
    if (auto xdgdd = getenv("XDG_DOCUMENTS_DIR"))
    {
        auto xdgpath = fs::path{xdgdd} / productName;
        return xdgpath;
    }

    auto hp = homePath();

    // If the user has already made one pick it
    auto documentpn = hp / "Documents" / productName;
    auto dotpn = hp / ("." + productName);

    if (fs::is_directory(dotpn))
        return dotpn;
    if (fs::is_directory(documentpn))
        return documentpn;

    // Neither is there. Do I have a Documents folder?
    if (auto documents = hp / "Documents"; fs::is_directory(documents))
        return documents / productName;
    return dotpn;
}

fs::path bestLibrarySharedFolderPathFor(const std::string &productName, bool userLevel)
{
    if (userLevel)
    {
        fs::path home = homePath();

        if (const char *xdgDataPath = getenv("XDG_DATA_HOME"))
        {
            return fs::path{xdgDataPath} / productName;
        }
        else
        {
            return home  / ".local" / "share" / productName;
        }
    }

    if (auto cmi = fs::path{CMAKE_INSTALL_PREFIX} / "share" / productName; fs::is_directory(cmi))
        return cmi;

    if (auto cmi = fs::path{"/usr"} / "share" / productName; fs::is_directory(cmi))
        return cmi;

    return fs::path{CMAKE_INSTALL_PREFIX} / "share" / productName;
}
}
}
}
