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

#ifndef INCLUDE_SST_PLUGININFRA_PATCH_SUPPORT_PATCH_BASE_H
#define INCLUDE_SST_PLUGININFRA_PATCH_SUPPORT_PATCH_BASE_H

#if SST_PLUGININFRA_PATCHBASE

#include <iostream>
#include <sstream>
#include <functional>
#include <type_traits>
#include <string>
#include <concepts>

#include "sst/basic-blocks/params/ParamMetadata.h"

#include "tinyxml/tinyxml.h"

namespace sst::plugininfra::patch_support
{

template <typename T>
concept ValidPar = requires(T p, float v) {
    { p.meta.id } -> std::convertible_to<uint32_t>;
    { p.meta.defaultVal } -> std::convertible_to<float>;
    p.value = v;
    { p.isTemposynced() } -> std::convertible_to<bool>;
};

template <typename Patch, typename Par>
concept ValidPatch = ValidPar<Par> && requires(Patch p, Par *par) {
    requires Patch::patchVersion > 0;
    requires Patch::id[0] != 0;
    { p.name[0] } -> std::same_as<char &>;
    { p.migratePatchFromVersion(uint32_t{}) } -> std::same_as<void>;
    { p.migrateParamValueFromVersion(par, float{}, uint32_t{}) } -> std::same_as<float>;
};

template <typename T>
concept PatchHasAuthor = requires(T &t) { t.author; };

using md_t = sst::basic_blocks::params::ParamMetaData;
struct ParamBase
{
    ParamBase(const md_t &m) : value(m.defaultVal), meta(m) {}
    float value{0};
    const md_t meta{};

    operator const float &() const { return value; }

    bool isTemposynced() const { return false; }
};

// Since this is used with a CRTP pattern, assert the validity concept
// inside the constructor not at declaration, since at declaration Patch and
// Par will be incomplete
template <typename Patch, typename Par> struct PatchBase
{
    bool dirty{false};
    std::vector<const Par *> params;
    std::unordered_map<uint32_t, Par *> paramMap;

    PatchBase() { static_assert(ValidPatch<Patch, Par>); }

    void pushSingleParam(Par *p)
    {
        params.push_back(p);
        if (paramMap.find(p->meta.id) != paramMap.end())
        {
            std::cerr << __FILE__ << ":" << __LINE__ << "Duplicate param id " << p->meta.id
                      << std::endl;
            std::cerr << __FILE__ << ":" << __LINE__ << " - New Param   : '" << p->meta.name << "'"
                      << std::endl;
            std::cerr << __FILE__ << ":" << __LINE__ << " - Other Param : '"
                      << paramMap[p->meta.id]->meta.name << "'" << std::endl;
        }
        paramMap.emplace(p->meta.id, p);
    }

    void pushSingleParam(Par *p, uint32_t id)
    {
        params.push_back(p);
        if (paramMap.find(id) != paramMap.end())
        {
            std::cerr << __FILE__ << ":" << __LINE__ << "Duplicate param id " << id << std::endl;
        }
        paramMap.emplace(id, p);
    }

    template <typename C> void pushMultipleParams(const C &c)
    {
        for (auto &p : c)
        {
            pushSingleParam(p);
        }
    }

    Patch *asPatch() { return static_cast<Patch *>(this); }
    const Patch *asPatch() const { return static_cast<const Patch *>(this); }

    std::string getAuthor() const
    {
        if constexpr (PatchHasAuthor<Patch>)
        {
            return std::string(asPatch()->author);
        }
        return {};
    }

    void setAuthor(const std::string &a)
    {
        if constexpr (PatchHasAuthor<Patch>)
        {
            using AuthorT = std::remove_reference_t<decltype(asPatch()->author)>;
            if constexpr (std::is_array_v<AuthorT>)
            {
                memset(asPatch()->author, 0, sizeof(asPatch()->author));
                strncpy(asPatch()->author, a.c_str(), sizeof(asPatch()->author) - 1);
            }
            else
            {
                asPatch()->author = a;
            }
        }
    }

    std::function<void(Patch &)> onResetToInit{nullptr};
    void resetToInit(const char *iname = "Init")
    {
        dirty = false;
        // Sweep any new param since this stream to default
        for (auto [id, p] : paramMap)
        {
            p->value = p->meta.defaultVal;
        }
        strncpy(asPatch()->name, "Init", 255);

        if (onResetToInit)
            onResetToInit(*asPatch());
    }

    std::function<void(TiXmlElement &)> additionalToState{nullptr};
    std::function<void(TiXmlElement *, uint32_t)> additionalFromState{nullptr};

    std::function<void(TiXmlElement &)> dawExtraStateTo{nullptr};
    std::function<void(TiXmlElement &)> dawExtraStateFrom{nullptr};

    std::string toState(bool withDawExtraState = false) const
    {
        TiXmlDocument doc;
        TiXmlElement rootNode("patch");
        rootNode.SetAttribute("id", Patch::id);
        rootNode.SetAttribute("version", Patch::patchVersion);
        rootNode.SetAttribute("name", asPatch()->name);
        if constexpr (PatchHasAuthor<Patch>)
        {
            rootNode.SetAttribute("author", getAuthor().c_str());
        }

        TiXmlElement paramsNode("params");

        for (auto p : params)
        {
            TiXmlElement param("p");
            param.SetAttribute("id", p->meta.id);
            param.SetDoubleAttribute("v", p->value);
            paramsNode.InsertEndChild(param);
        }

        rootNode.InsertEndChild(paramsNode);

        if (additionalToState)
            additionalToState(rootNode);

        if (withDawExtraState && dawExtraStateTo)
        {
            TiXmlElement dawExtraState("dawExtraState");
            dawExtraStateTo(dawExtraState);
            rootNode.InsertEndChild(dawExtraState);
        }

        doc.InsertEndChild(rootNode);

        std::ostringstream oss;
        oss << doc;
        return oss.str();
    }

    bool fromState(const std::string &idata)
    {
        resetToInit();

        TiXmlDocument doc;
        doc.Parse(idata.c_str());

        auto rn = doc.FirstChildElement("patch");
        if (!rn)
        {
            std::cerr << __FILE__ << ":" << __LINE__ << "No Patch Element" << std::endl;
            return false;
        }
        if (strcmp(rn->Attribute("id"), Patch::id) != 0)
        {
            std::cerr << __FILE__ << ":" << __LINE__ << "Wrong ID in patch file" << std::endl;
            return false;
        }
        int ver;
        if (rn->QueryIntAttribute("version", &ver) != TIXML_SUCCESS)
        {
            std::cerr << __FILE__ << ":" << __LINE__ << "No Version in Patch" << std::endl;
            return false;
        }

        if (rn->Attribute("name"))
        {
            memset(asPatch()->name, 0, sizeof(asPatch()->name));
            strncpy(asPatch()->name, rn->Attribute("name"), sizeof(asPatch()->name) - 1);
        }

        if constexpr (PatchHasAuthor<Patch>)
        {
            if (rn->Attribute("author"))
            {
                setAuthor(rn->Attribute("author"));
            }
            else
            {
                setAuthor("");
            }
        }

        auto pars = rn->FirstChildElement("params");
        if (!pars)
        {
            std::cerr << __FILE__ << ":" << __LINE__ << "No Params Element" << std::endl;
            return false;
        }

        auto *par = pars->FirstChildElement("p");
        while (par)
        {
            int id;
            double value;
            if (par->QueryIntAttribute("id", &id) == TIXML_SUCCESS &&
                par->QueryDoubleAttribute("v", &value) == TIXML_SUCCESS)
            {
                auto it = paramMap.find(id);
                if (it == paramMap.end())
                {
                    // SXSNLOG("  -  vestigal param " << id);
                }
                else
                {
                    auto *param = it->second;
                    value = asPatch()->migrateParamValueFromVersion(param, value, ver);
                    it->second->value = value;
                }
            }
            else
            {
                std::cerr << __FILE__ << ":" << __LINE__ << " Par missing id or value";
            }
            par = par->NextSiblingElement("p");
        }

        if (additionalFromState)
            additionalFromState(rn, ver);

        if (dawExtraStateFrom)
        {
            auto des = rn->FirstChildElement("dawExtraState");
            if (des)
                dawExtraStateFrom(*des);
        }

        if (ver != Patch::patchVersion)
            asPatch()->migratePatchFromVersion(ver);
        return true;
    }

    bool isParamTemposynced(Par *p) const { return p->isTemposynced(); }
};

} // namespace sst::plugininfra::patch_support

#endif
#endif // PATCH_BASE_H
