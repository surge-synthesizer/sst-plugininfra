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

#ifndef INCLUDE_SST_PLUGININFRA_KEYBINDINGS_H
#define INCLUDE_SST_PLUGININFRA_KEYBINDINGS_H

#include "tinyxml/tinyxml.h"
#include "filesystem/import.h"
#include <functional>
#include <optional>
#include <string>
#include <iostream>
#include <iomanip>
#include <map>
#include <unordered_map>
#include <sstream>
#include <fstream>

namespace sst
{
namespace plugininfra
{

template <typename KEY> inline std::string keyCodeToString(int keyCode)
{
    return std::to_string(keyCode);
}

template <typename KEY> inline int keyCodeFromString(const std::string &s)
{
    return std::atoi(s.c_str());
}

template <typename FUNCS, int maxFunc, typename KEY /* = juce::KeyPress */> struct KeyMapManager
{
    static constexpr int numFuncs = maxFunc;
    std::function<std::string(FUNCS)> enumToString;
    std::string productName;
    fs::path productPath;
    std::unordered_map<std::string, FUNCS> stringToEnum;
    KeyMapManager(const fs::path &productPath, const std::string &productName,
                  const std::function<std::string(FUNCS)> &e2S,
                  const std::function<void(const std::string &, const std::string &)> &errHandle)
        : enumToString(e2S), productPath(productPath), productName(productName)
    {
        for (int i = 0; i < maxFunc; ++i)
        {
            FUNCS f = (FUNCS)i;
            stringToEnum[enumToString(f)] = f;
        }
    }

    enum Modifiers : uint32_t
    {
        NONE = 0,
        SHIFT = 1 << 0,
        ALT = 1 << 1,
        COMMAND = 1 << 2,
        CONTROL = 1 << 3 // values are streamed
    };

    struct Binding
    {
        enum Type
        {
            INVALID = 0,
            TEXTCHAR = 1,
            KEYCODE = 2 // values are streamed
        } type{INVALID};

        uint32_t modifier{NONE};
        char textChar{0};
        int keyCode{0};

        bool active{true};

        Binding(uint32_t mod, char tc) : type(TEXTCHAR), textChar(tc), modifier(mod) {}
        Binding(uint32_t mod, int kc) : type(KEYCODE), keyCode(kc), modifier(mod) {}
        Binding(int kc) : type(KEYCODE), keyCode(kc), modifier(NONE) {}
        Binding() {}

        bool operator==(const Binding &other) const
        {
            if (modifier != other.modifier)
                return false;
            if (active != other.active)
                return false;
            if (type != other.type)
                return false;
            if (type == KEYCODE)
            {
                if (keyCode != other.keyCode)
                    return false;
            }
            else
            {
                if (textChar != other.textChar)
                    return false;
            }
            return true;
        }
        bool matches(const KEY &key) const
        {
            if (!active)
                return false;

            auto check = [this](uint32_t m, bool b) {
                if ((modifier & m) && !b)
                    return false;
                if (!(modifier & m) && b)
                    return false;
                return true;
            };

            auto kc = key.getKeyCode();
#if LINUX
            // Chording on linux is a mess. See  surge issue #6144
            if (kc >= 'a' && kc <= 'z')
            {
                kc = kc + 'A' - 'a';
            }
#endif

            if (!check(SHIFT, key.getModifiers().isShiftDown()))
#if LINUX
            {
                if (modifier != SHIFT && modifier != NONE && type == KEYCODE)
                {
                    /* this could be a linux user trying to upcase a key
                     * and chording is odd. Lets see if this fixes it
                     */
                }
                else
                {
                    return false;
                }
            }
#else
                return false;
#endif
#if SST_COMMAND_CTRL_SAME_KEY
            // Windows doesn't separate command and control gestures
            if (!check(COMMAND | CONTROL,
                       key.getModifiers().isCommandDown() || key.getModifiers().isCtrlDown()))
                return false;
#else
            if (!check(COMMAND, key.getModifiers().isCommandDown()))
                return false;
            if (!check(CONTROL, key.getModifiers().isCtrlDown()))
                return false;
#endif
            if (!check(ALT, key.getModifiers().isAltDown()))
                return false;

            if (type == KEYCODE && kc == keyCode)
                return true;
            if (type == TEXTCHAR && key.getTextCharacter() == textChar)
                return true;

            return false;
        }
    };

    std::map<FUNCS, Binding> bindings; // want this ordered for iteration display
    std::map<FUNCS, Binding> defaultBindings;
    void clearBindings()
    {
        bindings.clear();
        defaultBindings.clear();
    }
    void addBinding(const FUNCS &f, const Binding &&b)
    {
        bindings.emplace(f, b);
        // First one in is the default
        if (defaultBindings.find(f) == defaultBindings.end())
            defaultBindings.emplace(f, b);
    }
    std::optional<FUNCS> matches(const KEY &p)
    {
        for (const auto &[f, b] : bindings)
        {
            if (b.matches(p))
            {
                return {f};
            }
        }
        return {};
    }

    bool unstreamFromXML()
    {
        TiXmlDocument doc;
        if (!doc.LoadFile(productPath / (productName + "KeyboardMappings.xml")))
        {
            return false;
        }

        auto el = doc.FirstChildElement("keymappings");
        if (!el)
            return false;
        auto c = el->FirstChildElement();
        while (c)
        {
            FUNCS f;
            if (stringToEnum.find(c->Attribute("function")) != stringToEnum.end())
            {
                f = stringToEnum[c->Attribute("function")];
            }
            else
            {
                std::cerr << "Ignoring binding for unknown element " << c->Attribute("function")
                          << std::endl;
                c = c->NextSiblingElement();
                continue;
            }

            if (bindings.find(f) == bindings.end())
                bindings[f] = Binding();

            auto &b = bindings[f];

            int typeInt;
            if (c->Attribute("type", &(typeInt)))
            {
                b.type = static_cast<typename Binding::Type>(typeInt);
            }
            int ai;
            if (c->Attribute("active", &(ai)))
            {
                b.active = (bool)(ai);
            }
            int mod;
            if (c->Attribute("modifier", &(mod)))
            {
                b.modifier = static_cast<uint32_t>(mod);
            }

            auto tc = c->Attribute("textChar");
            if (tc && tc[0] != 0)
                b.textChar = tc[0];

            auto kc = c->Attribute("keyCode");
            if (kc)
                b.keyCode = keyCodeFromString<KEY>(kc);

            c = c->NextSiblingElement();
        }

        return true;
    }
    void streamToXML()
    {
        TiXmlDocument doc;
        doc.SetTabSize(4);
        TiXmlElement root("keymappings");

        for (const auto &[f, b] : bindings)
        {
            TiXmlElement bx("binding");
            bx.SetAttribute("function", enumToString(f));

            bx.SetAttribute("active", b.active);
            bx.SetAttribute("type", b.type);
            bx.SetAttribute("modifier", b.modifier);
            bx.SetAttribute("keyCode", keyCodeToString<KEY>(b.keyCode));

            char tc[2];
            tc[1] = 0;
            tc[0] = b.textChar;
            bx.SetAttribute("textChar", tc);
            root.InsertEndChild(bx);
        }

        doc.InsertEndChild(root);
        // inject the path_to_string to go down the xml path which formats.
        doc.SaveFile(path_to_string(productPath / (productName + "KeyboardMappings.xml")));
    }
};
} // namespace plugininfra
} // namespace sst

#endif // SURGE_KEYBINDINGS_H
