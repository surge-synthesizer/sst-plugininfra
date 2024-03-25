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

#ifndef INCLUDE_SST_PLUGININFRA_CPUFEATURES_H
#define INCLUDE_SST_PLUGININFRA_CPUFEATURES_H

#include <string>
#include <cstdint>

namespace sst
{
namespace plugininfra
{
namespace cpufeatures
{
std::string brand();

bool isArm();
bool isX86();
bool hasSSE2();
bool hasAVX();

struct FPUStateGuard
{
    FPUStateGuard();
    ~FPUStateGuard();

#if defined(__aarch64__)
    uint64_t priorS;
#else
    int priorS;
#endif
};
} // namespace cpufeatures
} // namespace plugininfra
} // namespace sst

#endif // SURGE_CPUFEATURES_H
