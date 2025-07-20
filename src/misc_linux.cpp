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

namespace sst
{
namespace plugininfra
{
namespace misc_platform
{
bool isDarkMode() { return true; }

void allocateConsole() {} // a mac/lin no-op but on windows it does stuff
} // namespace misc_platform
} // namespace plugininfra
} // namespace sst