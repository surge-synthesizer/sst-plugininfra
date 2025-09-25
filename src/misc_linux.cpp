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

// a macOS/LInux no-op, but on Windows it does stuff
void allocateConsole() {}

std::string toOSCase(const std::string &text) { return text; }
} // namespace misc_platform
} // namespace plugininfra
} // namespace sst