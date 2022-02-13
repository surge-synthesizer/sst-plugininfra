//
// Created by Paul Walker on 2/13/22.
//

#include <sst/plugininfra/misc_platform.h>
#include <CoreFoundation/CoreFoundation.h>
#include <Foundation/Foundation.h>

namespace sst
{
namespace plugininfra
{
namespace misc_platform
{
bool isDarkMode()
{
    NSString *interfaceStyle =
        [NSUserDefaults.standardUserDefaults valueForKey:@"AppleInterfaceStyle"];
    return [interfaceStyle isEqualToString:@"Dark"];
}
} // namespace misc_platform
} // namespace plugininfra
} // namespace sst