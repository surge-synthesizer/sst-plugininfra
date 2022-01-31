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

TEST_CASE("Documents Folder")
{
    SECTION("Got a documents folder")
    {
        auto dpath = sst::plugininfra::paths::bestDocumentsFolderPathFor("surge-test");
        std::cout << dpath.u8string() << std::endl;
        REQUIRE(true);
    }
}

TEST_CASE("FTZ", "[dsp]")
{
    auto g = sst::plugininfra::cpufeatures::FPUStateGuard();

    unsigned char min_float[4] = {0x00, 0x00, 0x80, 0x00};
    float f_min;
    memcpy(&f_min, min_float, 4);
    REQUIRE(f_min != 0.f);

    unsigned char res_float[4];
    for (int i = 0; i < 100; ++i)
    {
        // don't let the compiler optimize me away!
        auto r = 1.0 / (fabs(rand() % 10) + 1.1); // a number < 1
        float ftz_float = f_min * r;
        memcpy(res_float, &ftz_float, 4);
        for (int i = 0; i < 4; ++i)
        {
            REQUIRE(res_float[i] == 0);
        }
        REQUIRE(ftz_float == 0.f);
    }
}

int main(int argc, char **argv)
{
    int result = Catch::Session().run(argc, argv);
    return result;
}
