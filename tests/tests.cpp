#define CATCH_CONFIG_RUNNER
#include "catch2/catch2.hpp"

#include <sst/plugininfra.h>

TEST_CASE("Basic")
{
    SECTION("Base Test")
    {
        REQUIRE(sst::plugininfra::addOne(1) == 2);
    }
}

int main(int argc, char **argv)
{
    int result = Catch::Session().run(argc, argv);
    return result;
}
