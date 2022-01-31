
#ifndef SST_PLUGININFRA_CPUFEATURES_H
#define SST_PLUGININFRA_CPUFEATURES_H

#include <string>

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
