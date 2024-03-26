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
#include <cstdint>

#include <miniz.h>

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


    mz_zip_archive zip_archive;
    memset(&zip_archive, 0, sizeof(zip_archive));

    auto status = mz_zip_reader_init_file(&zip_archive, zipIn.u8string().c_str(), 0);
    REQUIRE(status);

    auto entries = (int)mz_zip_reader_get_num_files(&zip_archive);
    REQUIRE(entries == 3);

    for (int i = 0; i < (int)mz_zip_reader_get_num_files(&zip_archive); i++)
    {
        mz_zip_archive_file_stat file_stat;
        REQUIRE(mz_zip_reader_file_stat(&zip_archive, i, &file_stat));

        if (i == 0)
            REQUIRE(std::string(file_stat.m_filename) == "dumppaths.cpp");
        if (i == 1)
            REQUIRE(std::string(file_stat.m_filename) == "tests.cpp");
        if (i == 2)
            REQUIRE(std::string(file_stat.m_filename) == "tests_linux.cpp");
    }

}