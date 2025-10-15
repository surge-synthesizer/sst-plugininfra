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
#include <CoreFoundation/CoreFoundation.h>
#include <Foundation/Foundation.h>

#include <stdio.h>
#include <cstdlib>
#include <execinfo.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <cstring>

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

// a macOS/LInux no-op, but on Windows it does stuff
void allocateConsole() {}

std::string toOSCase(const std::string &text) { return text; }

std::string stackTraceToString(int depth)
{
    void *callstack[128];
    int i, frames = backtrace(callstack, 128);
    char **strs = backtrace_symbols(callstack, frames);
    if (depth < 0)
        depth = frames;
    std::ostringstream oss;
    oss << "-------- Stack Trace (" << depth << " frames of " << frames << " showing) --------\n";
    for (i = 1; i < frames && i < depth; ++i)
    {
        oss << "   [" << std::setw(3) << i << "]: " << strs[i] << "\n";
    }
    free(strs);
    return oss.str();
}

std::string getLastSystemError() { return std::string(strerror(errno)); }
} // namespace misc_platform
} // namespace plugininfra
} // namespace sst