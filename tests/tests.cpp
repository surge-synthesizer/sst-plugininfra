#define CATCH_CONFIG_RUNNER
#include "catch2/catch2.hpp"

#include <sst/plugininfra.h>
#include "filesystem/import.h"

TEST_CASE("FileSystem")
{
    SECTION("Can make a path")
    {
        auto p = fs::path{"tmp"} / fs::path{"var"};
        auto s = p.u8string();
        s[3] = ' ';
        REQUIRE(s == "tmp var");
    }
}

TEST_CASE( "Documents Folder" )
{
    SECTION( "Got a documents folder" )
    {
        auto dpath = sst::plugininfra::paths::bestDocumentsFolderPathFor("surge-test");
        std::cout << dpath.u8string() << std::endl;
        REQUIRE(true);
    }
}

int main(int argc, char **argv)
{
    int result = Catch::Session().run(argc, argv);
    return result;
}
