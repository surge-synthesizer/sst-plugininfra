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

#include <sst/plugininfra/misc_platform.h>
#include <tchar.h>
#include <windows.h>

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
} // namespace misc_platform
} // namespace plugininfra
} // namespace sst