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
 * An XDG base directory variable, or empty if unset. The basedir spec says an empty or
 * relative value must be treated as unset.
 */
static fs::path xdgBaseDir(const char *envName)
{
    const char *const value = getenv(envName);
    if (!value || !value[0])
        return fs::path{};
    auto p = fs::path{value};
    if (!p.is_absolute())
        return fs::path{};
    return p;
}

/*
 * Extract a custom user path defined in the file `user-dirs.dirs`.
 */
fs::path lookupXdgUserPath(const char *xdgDirId)
{
    fs::path userDirsPath;

    if (auto xdgConfigPath = xdgBaseDir("XDG_CONFIG_HOME"); !xdgConfigPath.empty())
    {
        userDirsPath = xdgConfigPath / "user-dirs.dirs";
    }
    else
    {
        userDirsPath = homePath() / ".config" / "user-dirs.dirs";
    }

#if SST_PLUGINFRA_GHC_FS
    fs::ifstream stream(userDirsPath);
#else
    std::ifstream stream(userDirsPath);
#endif
    return lookupXdgUserPathWithStream(xdgDirId, stream);
}

static bool isSameDirectory(const fs::path &a, const fs::path &b)
{
    // "/home/me/" and "/home/me" differ lexically, so drop the trailing separator
    auto norm = [](fs::path p) {
        p = p.lexically_normal();
        if (!p.has_filename())
            p = p.parent_path();
        return p;
    };
    if (norm(a) == norm(b))
        return true;
    std::error_code ec;
    return fs::equivalent(a, b, ec);
}

/*
 * The user's documents folder, or empty if there isn't one. xdg-user-dirs marks a
 * directory as disabled by setting it to $HOME, so that counts as not having one.
 */
static fs::path documentsRoot()
{
    auto hp = homePath();

    auto xdgdd = lookupXdgUserPath("XDG_DOCUMENTS_DIR");
    if (!xdgdd.empty() && !isSameDirectory(xdgdd, hp))
        return xdgdd;

    if (auto documents = hp / "Documents"; fs::is_directory(documents))
        return documents;
    return fs::path{};
}

/*
 * The waterfall we use here is as follows
 *
 * 1. Find the documents root: XDG_DOCUMENTS_DIR unless disabled, else an existing ~/Documents
 * 2. If root/vendor/product exists, use it
 * 3. If root/product exists, use it, so folders made without the vendor keep working
 * 4. If ~/.vendor/product exists, use it
 * 5. Otherwise root/vendor/product, or ~/.vendor/product if there is no documents root
 */
fs::path bestDocumentsVendorFolderPathFor(const std::string &vendorName,
                                          const std::string &productName)
{
    auto hp = homePath();
    auto root = documentsRoot();

    auto dotpn = hp / ("." + productName);
    if (!vendorName.empty())
        dotpn = hp / ("." + vendorName) / productName;

    if (!root.empty())
    {
        auto vendorpn = vendorName.empty() ? root / productName : root / vendorName / productName;
        if (fs::is_directory(vendorpn))
            return vendorpn;
        if (auto legacypn = root / productName; fs::is_directory(legacypn))
            return legacypn;
        if (fs::is_directory(dotpn))
            return dotpn;
        return vendorpn;
    }

    return dotpn;
}

fs::path bestLibrarySharedVendorFolderPathFor(const std::string &vendorName,
                                              const std::string &productName, bool userLevel)
{
    if (userLevel)
    {
        fs::path home = homePath();

        if (auto xdgDataPath = xdgBaseDir("XDG_DATA_HOME"); !xdgDataPath.empty())
        {
            if (!vendorName.empty())
                return xdgDataPath / vendorName / productName;
            else
                return xdgDataPath / productName;
        }
        else
        {
            if (!vendorName.empty())
                return home / ".local" / "share" / vendorName / productName;
            else
                return home / ".local" / "share" / productName;
        }
    }

    if (vendorName.empty())
    {
        if (auto cmi = fs::path{CMAKE_INSTALL_PREFIX} / "share" / productName;
            fs::is_directory(cmi))
            return cmi;

        if (auto cmi = fs::path{"/usr"} / "share" / productName; fs::is_directory(cmi))
            return cmi;

        return fs::path{CMAKE_INSTALL_PREFIX} / "share" / productName;
    }
    else
    {
        if (auto cmi = fs::path{CMAKE_INSTALL_PREFIX} / "share" / vendorName / productName;
            fs::is_directory(cmi))
            return cmi;

        if (auto cmi = fs::path{"/usr"} / "share" / vendorName / productName; fs::is_directory(cmi))
            return cmi;

        return fs::path{CMAKE_INSTALL_PREFIX} / "share" / vendorName / productName;
    }
}
} // namespace paths
} // namespace plugininfra
} // namespace sst
