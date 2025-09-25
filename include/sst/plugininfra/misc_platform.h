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

#ifndef INCLUDE_SST_PLUGININFRA_MISC_PLATFORM_H
#define INCLUDE_SST_PLUGININFRA_MISC_PLATFORM_H

#include <string>

namespace sst
{
namespace plugininfra
{
namespace misc_platform
{
bool isDarkMode();

// a macOS/LInux no-op, but on Windows it does stuff
void allocateConsole();

/*
 * Used to convert letter case for menu entries
 * Input string should be macOS style (Menu Entry is Like This)
 * Output is string in Windows style (Menu entry is like this), or unmodified if we're on macOS
 */
std::string toOSCase(const std::string &text);

/*
 * Some platforms can generate a stack trace as a string. Super useful
 * to debug. Of course if you use this in production code or tight loops
 * you have made a grave error. Beware.
 */
std::string stackTraceToString(int depth = -1);
} // namespace misc_platform
} // namespace plugininfra
} // namespace sst

#endif // SST_PLUGININFRA_OTHER_PLATFORM_H
