/*
 * paths_linux
 *
 * Semantics are:
 */

#include "filesystem/import.h"
#include "sst/plugininfra/paths.h"
#include <system_error>
#include <windows.h>
#include <shlobj.h>

namespace sst
{
namespace plugininfra
{
namespace paths
{

fs::path knownFolderPath(REFKNOWNFOLDERID rfid)
{
    fs::path path;
    PWSTR pathStr{};
    if (::SHGetKnownFolderPath(rfid, 0, nullptr, &pathStr) == S_OK)
        path = pathStr;
    ::CoTaskMemFree(pathStr);
    if (path.empty())
        throw std::runtime_error{"Failed to retrieve known folder path"};
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
    return knownFolderPath(FOLDERID_Documents) / productName;
}

fs::path bestLibrarySharedFolderPathFor(const std::string &productName, bool isUser)
{
    if (!isUser)
    {
        return knownFolderPath(FOLDERID_ProgramData) / productName;
    }
    else
    {
        return knownFolderPath(FOLDERID_LocalAppData) / productName;
    }
}
}
}
}
