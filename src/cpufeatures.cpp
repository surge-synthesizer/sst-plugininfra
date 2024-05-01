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
#include "sst/plugininfra/cpufeatures.h"

#if defined(__SSE2__) || defined(_M_AMD64) || defined(_M_X64) ||                                   \
    (defined(_M_IX86_FP) && _M_IX86_FP >= 2) || defined(__i386__) || defined(__x86_64__)
#define USING_X86 1
#include <xmmintrin.h>
#endif

#if MAC
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

#if LINUX
#include <fstream>
#endif

#ifdef _WIN32
#include <intrin.h>
#define cpuid(info, x) __cpuidex(info, x, 0)

#if !USING_X86
#error "Windows only works on X86 right now"
#endif
#else
#if LINUX && USING_X86
#ifdef __GNUC__
//  GCC Intrinsics
#include <cpuid.h>
void cpuid(int info[4], int InfoType)
{
    __cpuid_count(InfoType, 0, info[0], info[1], info[2], info[3]);
}
#endif
#endif
#endif

namespace sst
{
namespace plugininfra
{
namespace cpufeatures
{
std::string brand()
{
    std::string arch = "Unknown CPU";
#if MAC
    char buffer[1024];
    size_t bufsz = sizeof(buffer);
    if (sysctlbyname("machdep.cpu.brand_string", (void *)(&buffer), &bufsz, nullptr, (size_t)0) < 0)
    {
        arch = "Unknown";
    }
    else
    {
        arch = buffer;
    }

    if (sysctlbyname("hw.optional.arm64", (void *)(&buffer), &bufsz, nullptr, (size_t)0) == 0)
    {
        // So I am on an ARM but I am built both ways. Lets let folks know which one they run
#if defined(__aarch64__)
        // arch += " (Native)";
#else
        arch += " (Using Rosetta)";
#endif
    }

#elif WINDOWS
    std::string platform = "Windows";

    int CPUInfo[4] = {-1};
    unsigned nExIds, i = 0;
    char CPUBrandString[0x40];
    // Get the information associated with each extended ID.
    __cpuid(CPUInfo, 0x80000000);
    nExIds = CPUInfo[0];
    for (i = 0x80000000; i <= nExIds; ++i)
    {
        __cpuid(CPUInfo, i);
        // Interpret CPU brand string
        if (i == 0x80000002)
            memcpy(CPUBrandString, CPUInfo, sizeof(CPUInfo));
        else if (i == 0x80000003)
            memcpy(CPUBrandString + 16, CPUInfo, sizeof(CPUInfo));
        else if (i == 0x80000004)
            memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
    }
    arch = CPUBrandString;
#elif LINUX
    std::string platform = "Linux";

    // Lets see what /proc/cpuinfo has to say for us
    // on intels this is "model name"
    auto pinfo = std::ifstream("/proc/cpuinfo");
    if (pinfo.is_open())
    {
        std::string line;
        while (std::getline(pinfo, line))
        {
            if (line.find("model name") == 0)
            {
                auto colon = line.find(":");
                arch = line.substr(colon + 1);
                break;
            }
            if (line.find("Model") == 0) // rasperry pi branch
            {
                auto colon = line.find(":");
                arch = line.substr(colon + 1);
                break;
            }
        }
    }
    pinfo.close();
#endif
    return arch;
}

bool isArm()
{
#if defined(__arm__) || defined(__aarch64__)
    return true;
#else
    return false;
#endif
}
bool isX86()
{
#if USING_X86
    return true;
#else
    return false;
#endif
}
bool hasSSE2() { return true; }
bool hasAVX()
{
#if !USING_X86
    return true; // thanks simde
#elif MAC
    return true;
#elif WINDOWS || LINUX
    int info[4];
    cpuid(info, 0);
    unsigned int nIds = info[0];

    cpuid(info, 0x80000000);
    unsigned nExIds = info[0];

    bool avxSup = false;
    if (nIds >= 0x00000001)
    {
        cpuid(info, 0x00000001);

        avxSup = (info[2] & ((int)1 << 28)) != 0;
    }

    return avxSup;
#endif
}

FPUStateGuard::FPUStateGuard()
{
#if USING_X86
    auto _SSE_Flags = 0x8040;
    bool fpuExceptions = false;

    priorS = _mm_getcsr();
    if (fpuExceptions)
    {
        _mm_setcsr(((priorS & ~_MM_MASK_MASK) | _SSE_Flags) | _MM_EXCEPT_MASK); // all on
    }
    else
    {
        _mm_setcsr((priorS | _SSE_Flags) | _MM_MASK_MASK);
    }

    _MM_SET_ROUNDING_MODE(_MM_ROUND_NEAREST);
#endif

#if defined(__aarch64__)
    uint64_t FPCR = 0;
    asm volatile("MRS %0, FPCR " : "=r"(FPCR));

    uint64_t FPCR_new = FPCR | (1ULL << 24);
    asm volatile("MSR FPCR, %0 " : : "r"(FPCR_new));

    priorS = FPCR;
#endif
}

FPUStateGuard::~FPUStateGuard()
{
#if USING_X86
    _mm_setcsr(priorS);
#endif

#if defined(__aarch64__)
    asm volatile("MSR FPCR, %0 " : : "r"(priorS));
#endif
}
} // namespace cpufeatures
} // namespace plugininfra
} // namespace sst
