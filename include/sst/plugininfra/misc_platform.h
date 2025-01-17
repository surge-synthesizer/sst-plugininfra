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

#ifndef INCLUDE_SST_PLUGININFRA_MISC_PLATFORM_H
#define INCLUDE_SST_PLUGININFRA_MISC_PLATFORM_H

namespace sst
{
namespace plugininfra
{
namespace misc_platform
{
bool isDarkMode();
void allocateConsole(); // a mac/lin no-op but on windows it does stuff
} // namespace misc_platform
} // namespace plugininfra
} // namespace sst

#endif // SST_PLUGININFRA_OTHER_PLATFORM_H
