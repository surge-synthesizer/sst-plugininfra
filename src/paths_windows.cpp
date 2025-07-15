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

#include "filesystem/import.h"
#include "sst/plugininfra/paths.h"
#include <system_error>
#include <windows.h>
#include <shlobj.h>
#include <vector>
#include <string>
#include <sstream>

namespace sst
{
namespace plugininfra
{
namespace paths
{

fs::path knownFolderPath(REFKNOWNFOLDERID rfid, const std::string &idealName)
{
    fs::path path;
    PWSTR pathStr{};
    auto res = ::SHGetKnownFolderPath(rfid, 0, nullptr, &pathStr);
    if (res == S_OK)
        path = pathStr;
    ::CoTaskMemFree(pathStr);
    if (path.empty())
    {
        std::ostringstream oss;
        oss << "Failed to retrieve known folder path. Folder attempt is " << idealName
            << ". Error code is: " << std::system_category().message(res) << std::endl;
        throw std::runtime_error{oss.str()};
    }
    return path;
}

fs::path homePath() { return knownFolderPath(FOLDERID_Profile, "Home (Profile)"); }

fs::path sharedLibraryBinaryPath()
{
    HMODULE hmodule;
    if (!::GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                                  GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                              reinterpret_cast<LPCWSTR>(&sharedLibraryBinaryPath), &hmodule))
    {
        throw std::system_error{int(::GetLastError()), std::system_category(),
                                "Failed to retrieve module handle"};
    }

    for (std::vector<WCHAR> buf{MAX_PATH};; buf.resize(buf.size() * 2))
    {
        const auto len = ::GetModuleFileNameW(hmodule, &buf[0], buf.size());
        if (!len)
            throw std::system_error{int(::GetLastError()), std::system_category(),
                                    "Failed to retrieve module file name"};
        if (len < buf.size())
            return fs::path{&buf[0]};
    }
}

fs::path bestDocumentsVendorFolderPathFor(const std::string &vendorName,
                                          const std::string &productName)
{
    if (!vendorName.empty())
        return knownFolderPath(FOLDERID_Documents, "Documents") / vendorName / productName;
    else
        return knownFolderPath(FOLDERID_Documents, "Documents") / productName;
}

fs::path bestLibrarySharedVendorFolderPathFor(const std::string &vendorName,
                                              const std::string &productName, bool isUser)
{
    if (!isUser)
    {
        if (!vendorName.empty())
            return knownFolderPath(FOLDERID_ProgramData, "ProgramData") / vendorName / productName;
        else
            return knownFolderPath(FOLDERID_ProgramData, "ProgramData") / productName;
    }
    else
    {
        if (!vendorName.empty())
            return knownFolderPath(FOLDERID_LocalAppData, "LocalAppData") / vendorName /
                   productName;
        else
            return knownFolderPath(FOLDERID_LocalAppData, "LocalAppData") / productName;
    }
}
} // namespace paths
} // namespace plugininfra
} // namespace sst
