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

#ifndef INCLUDE_SST_PLUGININFRA_PATCH_SUPPORT_PATCH_BASE_CLAP_ADAPTER_H
#define INCLUDE_SST_PLUGININFRA_PATCH_SUPPORT_PATCH_BASE_CLAP_ADAPTER_H

#include <vector>
#include <cassert>
#include <string>
#include <cstdint>
#include <cstring>
#include <clap/clap.h>

namespace sst::plugininfra::patch_support
{
template <typename Patch>
inline bool patchToOutStream(Patch &patch, const clap_ostream *ostream) noexcept
{
    auto ss = patch.toState();
    auto c = ss.c_str();
    auto s = ss.length() + 1; // write the null terminator
    while (s > 0)
    {
        auto r = ostream->write(ostream, c, s);
        if (r < 0)
            return false;
        s -= r;
        c += r;
    }
    return true;
}

template <typename Patch>
inline bool inStreamToPatch(const clap_istream *istream, Patch &patch) noexcept
{
    static constexpr uint32_t initSize = 1 << 16, chunkSize = 1 << 8;
    std::vector<char> buffer;
    buffer.resize(initSize);

    int64_t rd{0};
    int64_t totalRd{0};
    auto bp = buffer.data();

    while ((rd = istream->read(istream, bp, chunkSize)) > 0)
    {
        bp += rd;
        totalRd += rd;
        if (totalRd >= buffer.size() - chunkSize - 1)
        {
            buffer.resize(buffer.size() * 2);
            bp = buffer.data() + totalRd;
        }
    }
    buffer[totalRd] = 0;

    if (totalRd == 0)
    {
        return false;
    }

    auto data = std::string(buffer.data());
    patch.fromState(data);
    return true;
}

template <typename Patch>
inline bool patchParamsInfo(uint32_t paramIndex, clap_param_info *info, Patch &patch) noexcept
{
    auto *param = patch.params[paramIndex];
    auto &md = param->meta;
    md.template toClapParamInfo<CLAP_NAME_SIZE, clap_param_info_t>(info);
    info->cookie = (void *)param;
    return true;
}

template <typename Patch>
inline bool patchParamsValue(clap_id paramId, double *value, const Patch &patch) noexcept
{
    if (patch.paramMap.find(paramId) == patch.paramMap.end())
        return false;
    *value = patch.paramMap.at(paramId)->value;
    return true;
}

template <typename Patch>
inline bool patchParamsValueToText(clap_id paramId, double value, char *display, uint32_t size,
                                   const Patch &patch) noexcept
{
    auto it = patch.paramMap.find(paramId);
    if (it == patch.paramMap.end())
    {
        return false;
    }
    auto valdisp = it->second->meta.valueToString(value);
    if (!valdisp.has_value())
        return false;

    strncpy(display, valdisp->c_str(), size);
    display[size - 1] = 0;
    return true;
}

template <typename Patch>
inline bool patchParamsTextToValue(clap_id paramId, const char *display, double *value,
                                   const Patch &patch) noexcept
{
    auto it = patch.paramMap.find(paramId);
    if (it == patch.paramMap.end())
    {
        return false;
    }
    std::string err;
    auto val = it->second->meta.valueFromString(display, err);
    if (!val.has_value())
    {
        return false;
    }
    *value = *val;
    return true;
}

template <typename Param, typename ClapEventType, typename Patch>
inline Param *paramFromClapEvent(const ClapEventType *pevt, const Patch &patch) noexcept
{
    auto *par = reinterpret_cast<Param *>(pevt->cookie);
    if (!par)
    {
        auto pid = pevt->param_id;
        auto pit = patch.paramMap.find(pid);
        if (pit == patch.paramMap.end())
            return nullptr;
        par = pit->second;
    }
    return par;
}

} // namespace sst::plugininfra::patch_support
#endif // PATCH_BASE_CLAP_ADAPTER_H
