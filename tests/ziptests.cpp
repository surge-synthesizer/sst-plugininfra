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

#include <iostream>

#include <zip.h>

#include "catch2/catch2.hpp"
#include "filesystem/import.h"

TEST_CASE("Can Unzip File")
{
    auto p = fs::current_path();
    while (p != p.root_directory() && !(fs::exists(p / "tests" / "three_cpp_files.zip")))
    {
        p = p.parent_path();
    }

    auto zipIn = p / "tests" / "three_cpp_files.zip";
    std::cout << "Reading rrom " << zipIn.u8string() << std::endl;

    int err;
    auto *zip = zip_open(zipIn.u8string().c_str(), ZIP_RDONLY, &err);
    REQUIRE(zip);

    auto entries = zip_get_num_entries(zip, 0);
    REQUIRE(entries == 3);
}