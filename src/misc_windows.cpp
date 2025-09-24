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

#include <sst/plugininfra/misc_platform.h>
#include <tchar.h>
#include <windows.h>
#include <iostream>
#include <stdio.h>

namespace sst
{
namespace plugininfra
{
namespace misc_platform
{
bool isDarkMode()
{
    DWORD value{};
    DWORD valueSize = sizeof(value);
    auto res = RegGetValue(HKEY_CURRENT_USER,
                           _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize"),
                           _T("AppsUseLightTheme"), RRF_RT_REG_DWORD, NULL, &value, &valueSize);
    if (res == ERROR_SUCCESS && value)
    {
        return false;
    }
    return true;
}

void allocateConsole()
{
    static FILE *confp{nullptr};
    AllocConsole();
    freopen_s(&confp, "CONOUT$", "w", stdout);
    HANDLE hConOut =
        CreateFile("CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                   NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    SetStdHandle(STD_OUTPUT_HANDLE, hConOut);
    std::cout.clear();
    std::wcout.clear();
}

std::string toOSCase(const std::string &text)
{
    auto s = text;

    for (auto i = 1; i < s.length() - 1; ++i)
    {
        if (!(std::isupper(s[i]) && (std::isupper(s[i + 1]) || !std::isalpha(s[i + 1]))))
        {
            s[i] = std::tolower(s[i]);
        }
    }

    return s;
}
} // namespace misc_platform
} // namespace plugininfra
} // namespace sst