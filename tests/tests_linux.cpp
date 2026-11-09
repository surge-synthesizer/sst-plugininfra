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
#include "catch2/catch2.hpp"
#include <sst/plugininfra.h>
#include <string>
#include <sstream>
#include <fstream>
#include <optional>
#include <vector>
#include <cstdlib>
#include <unistd.h>

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

namespace
{
// a throwaway $HOME with HOME and the XDG base vars restored on exit
struct FakeHome
{
    fs::path home;
    std::vector<std::pair<std::string, std::optional<std::string>>> saved;

    FakeHome()
    {
        home = fs::temp_directory_path() /
               ("sstpi-xdg-" + std::to_string(getpid()) + "-" + std::to_string(counter()++));
        fs::remove_all(home);
        fs::create_directories(home / ".config");
        for (auto v : {"HOME", "XDG_CONFIG_HOME", "XDG_DATA_HOME"})
        {
            auto c = getenv(v);
            saved.emplace_back(v, c ? std::optional<std::string>(c) : std::nullopt);
        }
        setenv("HOME", home.c_str(), 1);
        unsetenv("XDG_CONFIG_HOME");
        unsetenv("XDG_DATA_HOME");
    }
    ~FakeHome()
    {
        for (auto &[k, v] : saved)
        {
            if (v)
                setenv(k.c_str(), v->c_str(), 1);
            else
                unsetenv(k.c_str());
        }
        std::error_code ec;
        fs::remove_all(home, ec);
    }

    static int &counter()
    {
        static int c{0};
        return c;
    }

    void userDirs(const std::string &documents, const fs::path &configDir = {})
    {
        auto d = configDir.empty() ? home / ".config" : configDir;
        fs::create_directories(d);
        std::ofstream of(d / "user-dirs.dirs");
        of << "XDG_DOCUMENTS_DIR=\"" << documents << "\"\n";
    }

    void mkdir(const fs::path &rel) { fs::create_directories(home / rel); }
};

fs::path docs() { return sst::plugininfra::paths::bestDocumentsVendorFolderPathFor("V", "P"); }
} // namespace

TEST_CASE("Linux documents folder")
{
    namespace paths = sst::plugininfra::paths;

    SECTION("XDG documents dir gets the vendor folder")
    {
        FakeHome h;
        h.mkdir("Documents");
        h.userDirs("$HOME/Documents");
        REQUIRE(docs() == h.home / "Documents" / "V" / "P");
    }

    SECTION("Plain ~/Documents gets the vendor folder")
    {
        FakeHome h;
        h.mkdir("Documents");
        REQUIRE(docs() == h.home / "Documents" / "V" / "P");
    }

    SECTION("No documents at all uses the dot folder")
    {
        FakeHome h;
        REQUIRE(docs() == h.home / ".V" / "P");
    }

    SECTION("Disabled documents dir is not used as a root")
    {
        for (auto disabled : {"$HOME/", "$HOME"})
        {
            FakeHome h;
            h.userDirs(disabled);
            INFO("XDG_DOCUMENTS_DIR=" << disabled);
            REQUIRE(docs() == h.home / ".V" / "P");
        }
    }

    SECTION("Disabled documents dir falls back to an existing ~/Documents")
    {
        FakeHome h;
        h.mkdir("Documents");
        h.userDirs("$HOME/");
        REQUIRE(docs() == h.home / "Documents" / "V" / "P");
    }

    SECTION("Moved documents dir is followed")
    {
        FakeHome h;
        h.mkdir("Documents");
        h.userDirs("$HOME/elsewhere/docs", h.home / "cfg");
        setenv("XDG_CONFIG_HOME", (h.home / "cfg").c_str(), 1);
        REQUIRE(docs() == h.home / "elsewhere" / "docs" / "V" / "P");
    }

    SECTION("Existing folder without vendor is kept")
    {
        FakeHome h;
        h.mkdir("Documents/P");
        h.userDirs("$HOME/Documents");
        REQUIRE(docs() == h.home / "Documents" / "P");
    }

    SECTION("Existing vendor folder wins over one without")
    {
        FakeHome h;
        h.mkdir("Documents/P");
        h.mkdir("Documents/V/P");
        REQUIRE(docs() == h.home / "Documents" / "V" / "P");
    }

    SECTION("Existing dot folder is kept when Documents appears later")
    {
        FakeHome h;
        h.mkdir(".V/P");
        h.mkdir("Documents");
        REQUIRE(docs() == h.home / ".V" / "P");
    }

    SECTION("No vendor name")
    {
        FakeHome h;
        h.mkdir("Documents");
        REQUIRE(paths::bestDocumentsFolderPathFor("P") == h.home / "Documents" / "P");
    }

    SECTION("Empty or relative XDG_CONFIG_HOME is ignored")
    {
        for (auto bad : {"", "relative/config"})
        {
            FakeHome h;
            h.userDirs("$HOME/mine");
            setenv("XDG_CONFIG_HOME", bad, 1);
            INFO("XDG_CONFIG_HOME=" << bad);
            REQUIRE(docs() == h.home / "mine" / "V" / "P");
        }
    }

    SECTION("Empty or relative XDG_DATA_HOME is ignored")
    {
        for (auto bad : {"", "relative/data"})
        {
            FakeHome h;
            setenv("XDG_DATA_HOME", bad, 1);
            INFO("XDG_DATA_HOME=" << bad);
            REQUIRE(paths::bestLibrarySharedVendorFolderPathFor("V", "P", true) ==
                    h.home / ".local" / "share" / "V" / "P");
        }
    }

    SECTION("Absolute XDG_DATA_HOME is used")
    {
        FakeHome h;
        setenv("XDG_DATA_HOME", (h.home / "data").c_str(), 1);
        REQUIRE(paths::bestLibrarySharedVendorFolderPathFor("V", "P", true) ==
                h.home / "data" / "V" / "P");
    }
}
