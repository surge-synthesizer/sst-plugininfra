#define CATCH_CONFIG_RUNNER
#include "catch2/catch2.hpp"

#include <sst/plugininfra.h>
#include "filesystem/import.h"

TEST_CASE("Basic")
{
    SECTION("Base Test") { REQUIRE(sst::plugininfra::addOne(1) == 2); }
}

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

int main(int argc, char **argv)
{
    int result = Catch::Session().run(argc, argv);
    return result;
}
