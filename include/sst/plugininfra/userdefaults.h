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

#ifndef INCLUDE_SST_PLUGININFRA_USERDEFAULTS_H
#define INCLUDE_SST_PLUGININFRA_USERDEFAULTS_H

#include "tinyxml/tinyxml.h"
#include "filesystem/import.h"
#include <functional>
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
namespace defaults
{
/*
 * Given an enum provide user defaults. When constructed you also need to provide
 * a directory to store the defaults, a product name, and an enum-to-string function
 */
template <typename E, int maxE> struct Provider
{
    Provider(const fs::path &defaultsDirectory, const std::string &productName,
             const std::function<std::string(E)> &e2S,
             const std::function<void(const std::string &, const std::string &)> &errHandle)
        : defaultsDirectory(defaultsDirectory), productName(productName), enumToString(e2S),
          errorHandler(errHandle)
    {
        defaultsFile = defaultsDirectory / (productName + "UserDefaults.xml");
        initMaps();
        readDefaultsFile();
    }

    std::string getUserDefaultValue(const E &key, const std::string &valueIfMissing,
                                    bool potentiallyRead = true)
    {
        if (userPrefOverrides.find(key) != userPrefOverrides.end())
        {
            return userPrefOverrides[key].second;
        }

        if (potentiallyRead)
        {
            readDefaultsFile(false);
        }

        if (defaultsFileContents.find(key) != defaultsFileContents.end())
        {
            auto vStruct = defaultsFileContents[key];
            if (vStruct.type != UserDefaultValue::ud_string)
            {
                return valueIfMissing;
            }
            return vStruct.value;
        }

        return valueIfMissing;
    }

    int getUserDefaultValue(const E &key, int valueIfMissing, bool potentiallyRead = true)
    {
        if (userPrefOverrides.find(key) != userPrefOverrides.end())
        {
            return userPrefOverrides[key].first;
        }

        if (potentiallyRead)
        {
            readDefaultsFile(false);
        }

        if (defaultsFileContents.find(key) != defaultsFileContents.end())
        {
            auto vStruct = defaultsFileContents[key];
            if (vStruct.type != UserDefaultValue::ud_int)
            {
                return valueIfMissing;
            }
            return std::stoi(vStruct.value);
        }
        return valueIfMissing;
    }

    std::pair<int, int> getUserDefaultValue(const E &key, const std::pair<int, int> &valueIfMissing,
                                            bool potentiallyRead = true)
    {
        if (potentiallyRead)
        {
            readDefaultsFile(false);
        }

        if (defaultsFileContents.find(key) != defaultsFileContents.end())
        {
            auto vStruct = defaultsFileContents[key];
            if (vStruct.type != UserDefaultValue::ud_pair)
            {
                return valueIfMissing;
            }
            return vStruct.vpair;
        }
        return valueIfMissing;
    }

    fs::path getUserDefaultPath(const E &key, const fs::path &valueIfMissing)
    {
        return string_to_path(getUserDefaultValue(key, path_to_string(valueIfMissing)));
    }

    bool updateUserDefaultValue(const E &key, const std::string &value)
    {
        return storeUserDefaultValue(key, value, UserDefaultValue::ud_string);
    }

    bool updateUserDefaultValue(const E &key, const int value)
    {
        std::ostringstream oss;
        oss << value;
        return storeUserDefaultValue(key, oss.str(), UserDefaultValue::ud_int);
    }

    bool updateUserDefaultValue(const E &key, const std::pair<int, int> &value)
    {
        return storeUserDefaultValue(key, value, UserDefaultValue::ud_pair);
    }

    inline bool updateUserDefaultPath(const E &key, const fs::path &path)
    {
        return updateUserDefaultValue(key, path_to_string(path));
    }

    void addOverride(E key, const std::string &s) { userPrefOverrides[key] = {0, s}; }

    void addOverride(E key, int i) { userPrefOverrides[key] = {i, ""}; }

    void clearOverride(E key)
    {
        auto it = userPrefOverrides.find(key);
        if (it != userPrefOverrides.end())
            userPrefOverrides.erase(it);
    }

  private:
    std::function<std::string(E)> enumToString;
    std::function<void(const std::string &, const std::string &)> errorHandler;
    fs::path defaultsDirectory;
    fs::path defaultsFile;
    std::string productName;
    std::unordered_map<E, std::pair<int, std::string>> userPrefOverrides;

    struct UserDefaultValue
    {
        enum ValueType
        {
            ud_string = 1,
            ud_int = 2,
            ud_pair = 3
        } type;

        std::string keystring;
        E key;
        std::string value;
        std::pair<int, int> vpair;
    };

    void initMaps()
    {
        if (keysToStrings.empty())
        {
            // This is an odd form but it guarantees a compile error if we miss one
            for (int k = 0; k < maxE; ++k)
            {
                keysToStrings[(E)k] = enumToString((E)k);
            }

            for (const auto &p : keysToStrings)
                stringsToKeys[p.second] = p.first;
        }
    }

    // residual from surge codebase
    TiXmlElement *TINYXML_SAFE_TO_ELEMENT(TiXmlNode *n) { return n ? n->ToElement() : nullptr; }

    void readDefaultsFile(bool forceRead = false)
    {
        if (!haveReadDefaultsFile || forceRead)
        {
            initMaps();
            defaultsFileContents.clear();

            if (!fs::exists(defaultsFile))
                return;

            TiXmlDocument defaultsLoader;
            defaultsLoader.LoadFile(defaultsFile);
            TiXmlElement *e = TINYXML_SAFE_TO_ELEMENT(defaultsLoader.FirstChild("defaults"));
            if (e)
            {
                const char *version = e->Attribute("version");
                if (strcmp(version, "1") != 0)
                {
                    std::ostringstream oss;
                    oss << "This version of " << productName
                        << " reads only version 1 defaults. Your user "
                           "defaults version is "
                        << version << ". Defaults will be ignored!";
                    errorHandler(oss.str(), "File Version Error");
                    return;
                }

                TiXmlElement *def = TINYXML_SAFE_TO_ELEMENT(e->FirstChild("default"));
                while (def)
                {
                    UserDefaultValue v;
                    int vt;
                    def->Attribute("type", &vt);
                    v.type = (enum UserDefaultValue::ValueType)vt;
                    v.keystring = def->Attribute("key");

                    if (v.type == UserDefaultValue::ud_pair)
                    {
                        v.vpair.first = std::atoi(def->Attribute("firstvalue"));
                        v.vpair.second = std::atoi(def->Attribute("secondvalue"));
                    }
                    else
                    {
                        v.value = def->Attribute("value");
                    }

                    // silently disregard default keys we don't recognize
                    if (stringsToKeys.find(v.keystring) != stringsToKeys.end())
                    {
                        v.key = stringsToKeys[v.keystring];
                        defaultsFileContents[v.key] = v;
                    }

                    def = TINYXML_SAFE_TO_ELEMENT(def->NextSibling("default"));
                }
            }
            haveReadDefaultsFile = true;
        }
    }

    bool streamDefaultsFile()
    {
        /*
         ** For now, the format of our defaults file is so simple that we don't need to mess
         ** around with tinyxml to create it, just to parse it
         */
        std::ofstream dFile(defaultsFile);
        if (!dFile.is_open())
        {
            std::ostringstream emsg;
            emsg << "Unable to open defaults file '" << path_to_string(defaultsFile)
                 << "' for writing.";
            errorHandler(emsg.str(), "Defaults Not Saved");
            return false;
        }

        dFile << "<?xml version = \"1.0\" encoding = \"UTF-8\" ?>\n"
              << "<!-- User Defaults for Surge XT Synthesizer -->\n"
              << "<defaults version=\"1\">" << std::endl;

        for (auto &el : defaultsFileContents)
        {
            if (el.second.type == UserDefaultValue::ud_pair)
            {
                dFile << "  <default key=\"" << keysToStrings[el.first] << "\" firstvalue=\""
                      << el.second.vpair.first << "\" secondvalue=\"" << el.second.vpair.second
                      << "\" type=\"" << (int)el.second.type << "\"/>\n";
            }
            else
            {
                dFile << "  <default key=\"" << keysToStrings[el.first] << "\" value=\""
                      << el.second.value << "\" type=\"" << (int)el.second.type << "\"/>\n";
            }
        }

        dFile << "</defaults>" << std::endl;
        dFile.close();

        return true;
    }

    bool storeUserDefaultValue(const E &key, const std::pair<int, int> &val,
                               enum UserDefaultValue::ValueType type)
    {
        if (type != UserDefaultValue::ud_pair)
        {
            errorHandler("Software Error: Streamed pair as type non ud_pair", "UserDefaults");
            return false;
        }
        // Re-read the file in case another process] has updated it
        readDefaultsFile(true);

        try
        {
            // Create the directory if we need it
            fs::create_directories(defaultsDirectory);

            UserDefaultValue v;
            v.key = key;
            v.keystring = keysToStrings[key];
            v.vpair = val;
            v.type = type;

            defaultsFileContents[v.key] = v;
            return streamDefaultsFile();
        }
        catch (const fs::filesystem_error &e)
        {
            errorHandler(e.what(), "UserDefaults");
        }
        return false;
    }

    bool storeUserDefaultValue(const E &key, const std::string &val,
                               enum UserDefaultValue::ValueType type)
    {
        try
        {
            // Re-read the file in case another surge has updated it
            readDefaultsFile(true);

            fs::create_directories(defaultsDirectory);

            UserDefaultValue v;
            v.key = key;
            v.keystring = keysToStrings[key];
            v.value = val;
            v.type = type;

            defaultsFileContents[v.key] = v;
            return streamDefaultsFile();
        }
        catch (const fs::filesystem_error &e)
        {
            errorHandler(e.what(), "UserDefaults");
        }
        return false;
    }

    std::map<E, UserDefaultValue> defaultsFileContents;
    bool haveReadDefaultsFile{false};

    std::map<E, std::string> keysToStrings;
    std::map<std::string, E> stringsToKeys;
};
} // namespace defaults
} // namespace plugininfra
} // namespace sst

#endif // SHORTCIRCUIT_USERDEFAULTS_H
