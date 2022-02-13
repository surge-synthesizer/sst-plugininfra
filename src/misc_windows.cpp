//
// Created by Paul Walker on 2/13/22.
//

#include <sst/plugininfra/misc_platform.h>

namespace sst
{
namespace plugininfra
{
namespace misc_platform
{
bool isDarkMode()
{
#if 0
// Something like (although in C++ obviously)
private const string RegistryKeyPath = @"Software\Microsoft\Windows\CurrentVersion\Themes\Personalize";

private const string RegistryValueName = "AppsUseLightTheme";

private static ApplicationTheme GetWindowsTheme()
        {
            using var key = Registry.CurrentUser.OpenSubKey(RegistryKeyPath);
            var registryValueObject = key?.GetValue(RegistryValueName);
            if (registryValueObject == null)
            {
                return ApplicationTheme.Light;
            }
            var registryValue = (int)registryValueObject;

            return registryValue > 0 ? ApplicationTheme.Light : ApplicationTheme.Dark;
        }
#endif
    return true;
}
} // namespace misc_platform
} // namespace plugininfra
} // namespace sst