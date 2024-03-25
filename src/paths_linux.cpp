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

#include "filesystem/import.h"
#include "sst/plugininfra/paths.h"
#include <stdexcept>
#include <fstream>
#include <stdlib.h>
#include <string.h>
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
    if (!dladdr(reinterpret_cast<const void *>(&sharedLibraryBinaryPath), &info) ||
        !info.dli_fname[0])
    {
        // If dladdr(3) returns zero, dlerror(3) won't know why either
        throw std::runtime_error{"Failed to retrieve shared object file name"};
    }
    return fs::path{info.dli_fname};
}

/*
 * Extract a custom user path defined in the file `user-dirs.dirs`.
 * This auxiliary function which accepts an `istream` is for testing purposes.
 * The reference parser is found in Glib as `load_user_special_dirs`
 * (cf. `glib/gutils.c`).
 */
fs::path lookupXdgUserPathWithStream(const char *xdgDirId, std::istream &stream)
{
    if (stream)
    {
        std::string acc;
        acc.reserve(256);

        constexpr auto eof = std::char_traits<char>::eof();

        for (auto c = stream.get(); c != eof; c = stream.get())
        {
            // Skip leading space
            for (; c == ' ' || c == '\t'; c = stream.get())
                ;

            // Extract the variable name
            acc.clear();
            for (; c != eof && c != '=' && c != ' ' && c != '\t'; c = stream.get())
                acc.push_back(static_cast<unsigned char>(c));

            // Check it's our desired variable, otherwise discard
            if (acc != xdgDirId)
            {
                for (; c != eof && c != '\n'; c = stream.get())
                    ;
                continue;
            }

            // Skip space preceding '='
            for (; c == ' ' || c == '\t'; c = stream.get())
                ;

            // Expect '=' here, otherwise discard
            if (c != '=')
            {
                for (; c != eof && c != '\n'; c = stream.get())
                    ;
                continue;
            }
            c = stream.get();

            // Skip space following '='
            for (; c == ' ' || c == '\t'; c = stream.get())
                ;

            // Expect '"'
            if (c != '"')
            {
                for (; c != eof && c != '\n'; c = stream.get())
                    ;
                continue;
            }
            c = stream.get();

            // Extract the value
            acc.clear();
            for (; c != eof && c != '"' && c != '\n'; c = stream.get())
            {
                acc.push_back(static_cast<unsigned char>(c));
                if (acc.size() == 5 && !memcmp("$HOME", acc.data(), 5))
                    acc.assign(homePath().native());
            }

            // Expect '"'
            if (c != '"')
            {
                for (; c != eof && c != '\n'; c = stream.get())
                    ;
                continue;
            }

            // Found
            return fs::path{acc};
        }
    }

    return fs::path{};
}

/*
 * Extract a custom user path defined in the file `user-dirs.dirs`.
 */
fs::path lookupXdgUserPath(const char *xdgDirId)
{
    fs::path home = homePath();
    fs::path userDirsPath;

    if (const char *xdgConfigPath = getenv("XDG_CONFIG_HOME"))
    {
        userDirsPath = fs::path{xdgConfigPath} / "user-dirs.dirs";
    }
    else
    {
        userDirsPath = home / ".config" / "user-dirs.dirs";
    }

#if SST_PLUGINFRA_GHC_FS
    fs::ifstream stream(userDirsPath);
#else
    std::ifstream stream(userDirsPath);
#endif
    return lookupXdgUserPathWithStream(xdgDirId, stream);
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
    fs::path xdgdd = lookupXdgUserPath("XDG_DOCUMENTS_DIR");
    if (!xdgdd.empty())
    {
        auto xdgpath = xdgdd / productName;
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
            return home / ".local" / "share" / productName;
        }
    }

    if (auto cmi = fs::path{CMAKE_INSTALL_PREFIX} / "share" / productName; fs::is_directory(cmi))
        return cmi;

    if (auto cmi = fs::path{"/usr"} / "share" / productName; fs::is_directory(cmi))
        return cmi;

    return fs::path{CMAKE_INSTALL_PREFIX} / "share" / productName;
}
} // namespace paths
} // namespace plugininfra
} // namespace sst
