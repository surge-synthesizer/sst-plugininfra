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
#include "catch2/catch2.hpp"
#include <sst/plugininfra.h>
#include <string>
#include <sstream>

namespace sst
{
namespace plugininfra
{
namespace paths
{
fs::path lookupXdgUserPathWithStream(const char *xdgDirId, std::istream &stream);
}
} // namespace plugininfra
} // namespace sst

TEST_CASE("Linux user directories")
{
    namespace paths = sst::plugininfra::paths;

    const std::string userDirsText = "XDG_DESKTOP_DIR=\"$HOME/MyDesktop\""
                                     "\n"
                                     "XDG_DOCUMENTS_DIR=\"$HOME/MyDocuments\""
                                     "\n"
                                     "XDG_DOWNLOAD_DIR=\"/MyDownloads\""
                                     "\n";

    auto lookup = [&userDirsText](const char *name) -> fs::path {
        std::istringstream stream(userDirsText);
        return paths::lookupXdgUserPathWithStream(name, stream);
    };

    fs::path home = paths::homePath();

    REQUIRE(lookup("XDG_DESKTOP_DIR") == home / "MyDesktop");
    REQUIRE(lookup("XDG_DOCUMENTS_DIR") == home / "MyDocuments");
    REQUIRE(lookup("XDG_DOWNLOAD_DIR") == "/MyDownloads");
    REQUIRE(lookup("XDG_MUSIC_DIR").empty());
}
