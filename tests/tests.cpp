#define CATCH_CONFIG_RUNNER
#include "catch2/catch2.hpp"

#include <sst/plugininfra.h>
#include <sst/plugininfra/userdefaults.h>
#include <sst/plugininfra/strnatcmp.h>
#include "sst/plugininfra/keybindings.h"
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

TEST_CASE("Defaults")
{
    enum K
    {
        Foo,
        Bar,
        Three,
        Four,
        nK
    };
    auto k2s = [](K k) -> std::string {
        if (k == Foo)
            return "Foo";
        if (k == Bar)
            return "Bar";
        if (k == Three)
            return "Three";
        if (k == Four)
            return "Four";
        return "Blugh";
    };
    using P = sst::plugininfra::defaults::Provider<K, nK>;

    SECTION("Write and Read")
    {
        int di = 0;
        auto td = fs::temp_directory_path() / ("tst_" + std::to_string(di));
        while (fs::exists(td) && di < 1000)
        {
            di++;
            td = fs::temp_directory_path() / ("tst_" + std::to_string(di));
        }
        REQUIRE(di < 1000);
        fs::create_directories(td);
        INFO("Making test in " << td.u8string());
        auto p1 = P(td, "TestCase", k2s, [](auto a, auto b) {});
        REQUIRE(p1.getUserDefaultValue(Foo, 1) == 1);
        REQUIRE(p1.getUserDefaultValue(Bar, "bb") == "bb");
        REQUIRE(p1.getUserDefaultValue(Three, std::pair{2, 2}) == std::pair{2, 2});
        REQUIRE(p1.getUserDefaultPath(Four, td) == td);

        p1.updateUserDefaultValue(Foo, 2);
        p1.updateUserDefaultValue(Bar, "qq");
        p1.updateUserDefaultValue(Three, std::pair{7, 9});
        auto nd = td / ("chg" + std::to_string(rand()));
        p1.updateUserDefaultPath(Four, nd);

        REQUIRE(p1.getUserDefaultValue(Foo, 1) == 2);
        REQUIRE(p1.getUserDefaultValue(Bar, "bb") == "qq");
        REQUIRE(p1.getUserDefaultValue(Three, std::pair{2, 2}) == std::pair{7, 9});
        REQUIRE(p1.getUserDefaultPath(Four, td) == nd);
    }

    SECTION("Write and Read Cross Instances")
    {
        int di = 0;
        auto td = fs::temp_directory_path() / ("tst_" + std::to_string(di));
        while (fs::exists(td) && di < 1000)
        {
            di++;
            td = fs::temp_directory_path() / ("tst_" + std::to_string(di));
        }
        auto nd = td / ("chg" + std::to_string(rand()));

        REQUIRE(di < 1000);
        fs::create_directories(td);
        INFO("Making test in " << td.u8string());
        {
            auto p1 = P(td, "TestCase", k2s, [](auto a, auto b) {});
            REQUIRE(p1.getUserDefaultValue(Foo, 1) == 1);
            REQUIRE(p1.getUserDefaultValue(Bar, "bb") == "bb");
            REQUIRE(p1.getUserDefaultValue(Three, std::pair{2, 2}) == std::pair{2, 2});
            REQUIRE(p1.getUserDefaultPath(Four, td) == td);

            p1.updateUserDefaultValue(Foo, 2);
            p1.updateUserDefaultValue(Bar, "qq");
            p1.updateUserDefaultValue(Three, std::pair{7, 9});
            p1.updateUserDefaultPath(Four, nd);

            REQUIRE(p1.getUserDefaultValue(Foo, 1) == 2);
            REQUIRE(p1.getUserDefaultValue(Bar, "bb") == "qq");
            REQUIRE(p1.getUserDefaultValue(Three, std::pair{2, 2}) == std::pair{7, 9});
            REQUIRE(p1.getUserDefaultPath(Four, td) == nd);
        }

        auto p2 = P(td, "TestCase", k2s, [](auto a, auto b) {});
        REQUIRE(p2.getUserDefaultValue(Foo, 1) == 2);
        REQUIRE(p2.getUserDefaultValue(Bar, "bb") == "qq");
        REQUIRE(p2.getUserDefaultValue(Three, std::pair{2, 2}) == std::pair{7, 9});
        REQUIRE(p2.getUserDefaultPath(Four, td) == nd);
    }
}

TEST_CASE("strnatcmp with spaces", "[infra]")
{
    SECTION("Basic Compare")
    {
        REQUIRE(strnatcmp("foo", "bar") == 1);
        REQUIRE(strnatcmp("bar", "foo") == -1);
        REQUIRE(strnatcmp("bar", "bar") == 0);
    }

    SECTION("Number Compare")
    {
        REQUIRE(strnatcmp("1 foo", "2 foo") == -1);
        REQUIRE(strnatcmp("1 foo", "11 foo") == -1);
        REQUIRE(strnatcmp("1 foo", "91 foo") == -1);
        REQUIRE(strnatcmp("01 foo", "91 foo") == -1);
        REQUIRE(strnatcmp("91 foo", "1 foo") == 1);
        REQUIRE(strnatcmp("91 foo", "01 foo") == 1);
        REQUIRE(strnatcmp("91 foo", "001 foo") == 1);
        REQUIRE(strnatcmp("1 foo", "112 foo") == -1);
        REQUIRE(strnatcmp("91 foo", "112 foo") == -1);
        REQUIRE(strnatcmp("112 foo", "91 foo") == 1);
        REQUIRE(strnatcmp("01 foo", "2 foo") == -1);
        REQUIRE(strnatcmp("001 foo", "2 foo") == -1);
        REQUIRE(strnatcmp("001 foo", "002 foo") == -1);
        // Fix these one day
        // REQUIRE(strnatcmp("01 foo", "002 foo") == -1);
        // REQUIRE(strnatcmp("1 foo", "002 foo") == -1);
        // REQUIRE(strnatcmp("1 foo", "02 foo") == -1);
    }

    SECTION("Spaces vs Letters")
    {
        REQUIRE(strnatcmp("hello", "zebra") == -1);
        REQUIRE(strnatcmp("hello ", "zebra") == -1);
        REQUIRE(strnatcmp(" hello", "zebra") == -1);
        REQUIRE(strnatcmp("hello", " zebra") == -1);
        REQUIRE(strnatcmp("hello", "zebra ") == -1);
        REQUIRE(strnatcmp("hello", "z") == -1);
        REQUIRE(strnatcmp("hello", "z ") == -1);
        REQUIRE(strnatcmp("hello", " z") == -1);
        REQUIRE(strnatcmp(" hello", "z") == -1);
        REQUIRE(strnatcmp("hello ", "z") == -1);
    }

    SECTION("Central Spaces")
    {
        REQUIRE(strnatcmp("Spa Day", "SpaDay") == -1);
        REQUIRE(strnatcmp("SpaDay", "Spa Day") == 1);
    }
    SECTION("Doubled Spaces") { REQUIRE(strnatcmp("Spa  Day", "Spa Day") == 0); }
}

TEST_CASE("Dark Mode Returns Something")
{
    bool dm = sst::plugininfra::misc_platform::isDarkMode();
    std::cout << "DARK MODE IS " << dm << std::endl;
    REQUIRE(true);
}

TEST_CASE("KeyBindings Compile Absent JUCE")
{
    enum Foo
    {
        FIRST,
        SECOND,
        THIRD
    };
    struct TestKeyPress
    {
        struct M
        {
            bool isCtrlDown() const { return false; }
            bool isAltDown() const { return false; }
            bool isShiftDown() const { return false; }
            bool isCommandDown() const { return false; }
        };
        M getModifiers() const { return M(); }
        char getTextCharacter() const { return ' '; }
        int getKeyCode() const { return 0; }
    };
    typedef sst::jucepluginfra::KeyMapManager<Foo, 3, TestKeyPress> keymap_t;
    auto km = std::make_unique<keymap_t>(
        fs::path(), "TestProduct", [](Foo f) { return ""; }, [](auto a, auto b) {});
    REQUIRE(km);

    km->addBinding(FIRST, {'S'});
    km->matches(TestKeyPress());
}

int main(int argc, char **argv)
{
    int result = Catch::Session().run(argc, argv);
    return result;
}
