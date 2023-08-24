/*
 * paths_windows
 *
 * Semantics are:
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
        std::ostringstream  oss;
        oss << "Failed to retrieve known folder path. Folder attempt is " << idealName << ". Error code is " << res << std::endl;
        throw std::runtime_error{oss.str()};
    }
    return path;
}


fs::path homePath()
{
    return knownFolderPath(FOLDERID_Profile);
}

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

fs::path bestDocumentsFolderPathFor(const std::string &productName)
{
    return knownFolderPath(FOLDERID_Documents, "Documents") / productName;
}

fs::path bestLibrarySharedFolderPathFor(const std::string &productName, bool isUser)
{
    if (!isUser)
    {
        return knownFolderPath(FOLDERID_ProgramData, "ProgramData") / productName;
    }
    else
    {
        return knownFolderPath(FOLDERID_LocalAppData, "LocalAppData") / productName;
    }
}
}
}
}
