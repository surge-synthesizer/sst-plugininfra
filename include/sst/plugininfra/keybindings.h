// /*
//  ** Surge Synthesizer is Free and Open Source Software
//  **
//  ** Surge is made available under the Gnu General Public License, v3.0
//  ** https://www.gnu.org/licenses/gpl-3.0.en.html
//  **
//  ** Copyright 2004-2021 by various individuals as described by the Git transaction log
//  **
//  ** All source at: https://github.com/surge-synthesizer/surge.git
//  **
//  ** Surge was a commercial product from 2004-2018, with Copyright and ownership
//  ** in that period held by Claes Johanson at Vember Audio. Claes made Surge
//  ** open source in September 2018.
//  *

//
// Created by Paul Walker on 3/21/22.
//

#ifndef SURGE_KEYBINDINGS_H
#define SURGE_KEYBINDINGS_H

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
namespace jucepluginfra
{

template <typename FUNCS, int maxFunc, typename KEY /* = juce::KeyPress */> struct KeyMapManager
{
    std::function<std::string(FUNCS)> enumToString;
    KeyMapManager(const fs::path &defaultsDirectory, const std::string &productName,
                  const std::function<std::string(FUNCS)> &e2S,
                  const std::function<void(const std::string &, const std::string &)> &errHandle)
        : enumToString(e2S)
    {
    }

    enum Modifiers
    {
        NONE = 0,
        SHIFT = 1 << 0,
        ALT = 1 << 1,
        COMMAND = 1 << 2,
        CONTROL = 1 << 3
    };

    struct Binding
    {
        enum Type
        {
            TEXTCHAR,
            KEYCODE
        } type;

        Modifiers modifier{NONE};
        char textChar{0};
        int keyCode{0};

        bool active{true};

        Binding(Modifiers mod, char tc) : type(TEXTCHAR), textChar(tc), modifier(mod) {}
        Binding(Modifiers mod, int kc) : type(KEYCODE), keyCode(kc), modifier(mod) {}
        Binding(int kc) : type(KEYCODE), keyCode(kc), modifier(NONE) {}

        bool matches(const KEY &key) const
        {
            if (!active)
                return false;

            switch (modifier)
            {
            case SHIFT:
                if (!key.getModifiers().isShiftDown())
                    return false;
                break;
            case COMMAND:
                if (!key.getModifiers().isCommandDown())
                    return false;
                break;
            case CONTROL:
                if (!key.getModifiers().isCtrlDown())
                    return false;
                break;
            case ALT:
                if (!key.getModifiers().isAltDown())
                    return false;
                break;
            case NONE:
                break;
            }

            if (type == KEYCODE && key.getKeyCode() == keyCode)
                return true;
            if (type == TEXTCHAR && key.getTextCharacter() == textChar)
                return true;

            return false;
        }
    };

    std::unordered_map<FUNCS, Binding> bindings;
    void addBinding(const FUNCS &f, const Binding &&b) { bindings.emplace(f, b); }
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

    void unstreamFromXML() {}
    void streamToXML() {}
};
} // namespace jucepluginfra
} // namespace sst

#endif // SURGE_KEYBINDINGS_H
