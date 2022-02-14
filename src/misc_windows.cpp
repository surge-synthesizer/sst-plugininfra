//
// Created by Paul Walker on 2/13/22.
//

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
                _T("AppsUseLightTheme"),
                RRF_RT_REG_DWORD,
                NULL,
                &value,
                &valueSize);
    if (res == ERROR_SUCCESS && value)
    {
        return false;
    }
    return true;
}
} // namespace misc_platform
} // namespace plugininfra
} // namespace sst