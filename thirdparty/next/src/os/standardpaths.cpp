/*
    This file is part of Thunder Next.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "standardpaths.h"

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#include <knownfolders.h>
#include <string>
#elif defined(__APPLE__)
#include <cstdlib>
#include <string>
#else
#include <cstdlib>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <string>
#include <fstream>
#endif

/*!
    \class StandardPaths
    \brief Helper class for querying standard system locations.
    \since Next 1.0
    \inmodule OS

    Platform-aware helper for resolving common filesystem locations such as
    application data, cache, config, documents, downloads, music, pictures,
    videos, temporary and runtime directories.

    On Windows it relies on the Known Folder API (`SHGetKnownFolderPath`)
    and `GetEnvironmentVariableW`. On macOS it uses
    `NSSearchPathForDirectoriesInDomains` and `NSTemporaryDirectory`
    (implemented in `standardpaths.mm`).
    On Linux/BSD it follows the XDG Base Directory Specification and
    XDG user directories.
*/

#ifdef _WIN32
namespace {

std::wstring toWide(const TString &str) {
    const std::string &s = str.toStdString();
    if(s.empty()) {
        return std::wstring();
    }
    int size = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    std::wstring result(size > 0 ? size - 1 : 0, L'\0');
    if(size > 0) {
        MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &result[0], size);
        result.resize(size - 1);
    }
    return result;
}

TString fromWide(const std::wstring &str) {
    if(str.empty()) {
        return TString();
    }
    int size = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1,
                                   nullptr, 0, nullptr, nullptr);
    std::string result(size > 0 ? size - 1 : 0, '\0');
    if(size > 0) {
        WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1,
                            &result[0], size, nullptr, nullptr);
        result.resize(size - 1);
    }
    return TString(result);
}

TString knownFolder(REFKNOWNFOLDERID id) {
    PWSTR path = nullptr;
    if(SUCCEEDED(SHGetKnownFolderPath(id, 0, nullptr, &path))) {
        std::wstring w(path);
        CoTaskMemFree(path);
        return fromWide(w);
    }
    return TString();
}

TString envVar(const wchar_t *name) {
    DWORD size = GetEnvironmentVariableW(name, nullptr, 0);
    if(size == 0) {
        return TString();
    }
    std::wstring buffer(size, L'\0');
    GetEnvironmentVariableW(name, &buffer[0], size);
    if(!buffer.empty() && buffer.back() == L'\0') {
        buffer.pop_back();
    }
    return fromWide(buffer);
}

} // namespace
#endif // _WIN32

#ifdef __APPLE__
// Реализация — в standardpaths.mm (Objective-C++).
extern "C" {

// Возвращают malloc-строку UTF-8, которую надо освободить free().
// dir — значение NSSearchPathDirectory (см. константы ниже).
char *sp_mac_search_path(int dir);
char *sp_mac_temporary_directory();
char *sp_mac_home_directory();

} // extern "C"

namespace {

// Значения NSSearchPathDirectory стабильны и не меняются между SDK.
enum {
    kNSDocumentDirectory           = 9,
    kNSApplicationSupportDirectory = 14,
    kNSCachesDirectory             = 13,
    kNSDownloadsDirectory          = 15,
    kNSMusicDirectory              = 18,
    kNSPicturesDirectory           = 19,
    kNSMoviesDirectory             = 17
};

// Оборачивает char* в TString и освобождает память.
TString takeCString(char *raw) {
    if(!raw) {
        return TString();
    }
    TString result(raw);
    std::free(raw);
    return result;
}

TString nsPath(int dir) {
    return takeCString(sp_mac_search_path(dir));
}

TString homeDir() {
    return takeCString(sp_mac_home_directory());
}

} // namespace
#endif // __APPLE__

#if !defined(_WIN32) && !defined(__APPLE__)
namespace {

TString homeDir() {
    const char *home = std::getenv("HOME");
    if(home && *home) {
        return TString(home);
    }
    struct passwd *pw = getpwuid(getuid());
    if(pw && pw->pw_dir) {
        return TString(pw->pw_dir);
    }
    return TString();
}

TString xdgVar(const char *name, const TString &fallback) {
    const char *value = std::getenv(name);
    if(value && *value) {
        return TString(value);
    }
    return fallback;
}

// Expand values like "$HOME/Downloads" taken from user-dirs.dirs.
TString expandUserDir(const std::string &value) {
    if(value.empty()) {
        return TString();
    }
    if(value[0] != '$') {
        return TString(value);
    }
    size_t slash = value.find('/');
    std::string var = value.substr(1, slash == std::string::npos
                                          ? std::string::npos
                                          : slash - 1);
    const char *env = std::getenv(var.c_str());
    TString base = env ? TString(env) : TString();
    if(slash == std::string::npos) {
        return base;
    }
    return base + value.substr(slash).c_str();
}

TString userDir(const char *key, const TString &fallback) {
    TString configHome = xdgVar("XDG_CONFIG_HOME", homeDir() + "/.config");
    TString file = configHome + "/user-dirs.dirs";

    std::ifstream in(file.toStdString().c_str());
    if(in.is_open()) {
        std::string line;
        std::string prefix = std::string(key) + "=\"";
        while(std::getline(in, line)) {
            if(line.compare(0, prefix.size(), prefix) == 0) {
                size_t end = line.rfind('"');
                if(end != std::string::npos && end > prefix.size()) {
                    return expandUserDir(line.substr(prefix.size(),
                                                     end - prefix.size()));
                }
            }
        }
    }
    return fallback;
}

void splitPaths(const TString &list, StringList &out) {
    const std::string &s = list.toStdString();
    size_t start = 0;
    while(true) {
        size_t pos = s.find(':', start);
        if(pos == std::string::npos) {
            if(start < s.size()) {
                out.push_back(TString(s.substr(start)));
            }
            break;
        }
        if(pos > start) {
            out.push_back(TString(s.substr(start, pos - start)));
        }
        start = pos + 1;
    }
}

} // namespace
#endif // POSIX

/*! Return the first writable location for \a type. */
TString StandardPaths::writableLocation(StandardLocation type) {
#ifdef _WIN32
    switch(type) {
    case ApplicationsLocation:  return knownFolder(FOLDERID_ProgramFiles);
    case DocumentsLocation:     return knownFolder(FOLDERID_Documents);
    case CacheLocation:         return knownFolder(FOLDERID_LocalAppData);
    case AppDataLocation:
    case AppLocalDataLocation:  return knownFolder(FOLDERID_LocalAppData);
    case ConfigLocation:        return knownFolder(FOLDERID_RoamingAppData);
    case DownloadLocation:      return knownFolder(FOLDERID_Downloads);
    case MusicLocation:         return knownFolder(FOLDERID_Music);
    case PicturesLocation:      return knownFolder(FOLDERID_Pictures);
    case VideosLocation:        return knownFolder(FOLDERID_Videos);
    case TempLocation:          return envVar(L"TEMP");
    case HomeLocation:          return knownFolder(FOLDERID_Profile);
    case RuntimeLocation:       return envVar(L"LOCALAPPDATA");
    }
#elif defined(__APPLE__)
    switch(type) {
    case ApplicationsLocation:  return TString("/Applications");
    case DocumentsLocation:     return nsPath(kNSDocumentDirectory);
    case CacheLocation:         return nsPath(kNSCachesDirectory);
    case AppDataLocation:
    case AppLocalDataLocation:  return nsPath(kNSApplicationSupportDirectory);
    case ConfigLocation:        return homeDir() + "/Library/Preferences";
    case DownloadLocation:      return nsPath(kNSDownloadsDirectory);
    case MusicLocation:         return nsPath(kNSMusicDirectory);
    case PicturesLocation:      return nsPath(kNSPicturesDirectory);
    case VideosLocation:        return nsPath(kNSMoviesDirectory);
    case TempLocation:          return takeCString(sp_mac_temporary_directory());
    case HomeLocation:          return homeDir();
    case RuntimeLocation:       return nsPath(kNSCachesDirectory);
    }
#else
    TString home = homeDir();
    switch(type) {
    case ApplicationsLocation:  return TString("/usr/share");
    case DocumentsLocation:     return userDir("XDG_DOCUMENTS_DIR", home + "/Documents");
    case CacheLocation:         return xdgVar("XDG_CACHE_HOME", home + "/.cache");
    case AppDataLocation:
    case AppLocalDataLocation:  return xdgVar("XDG_DATA_HOME", home + "/.local/share");
    case ConfigLocation:        return xdgVar("XDG_CONFIG_HOME", home + "/.config");
    case DownloadLocation:      return userDir("XDG_DOWNLOAD_DIR", home + "/Downloads");
    case MusicLocation:         return userDir("XDG_MUSIC_DIR", home + "/Music");
    case PicturesLocation:      return userDir("XDG_PICTURES_DIR", home + "/Pictures");
    case VideosLocation:        return userDir("XDG_VIDEOS_DIR", home + "/Videos");
    case TempLocation: {
        const char *tmp = std::getenv("TMPDIR");
        return (tmp && *tmp) ? TString(tmp) : TString("/tmp");
    }
    case HomeLocation:          return home;
    case RuntimeLocation: {
        const char *xdg = std::getenv("XDG_RUNTIME_DIR");
        return (xdg && *xdg) ? TString(xdg) : TString("/tmp");
    }
    }
#endif
    return TString();
}

/*! Return all standard locations for \a type, most relevant first. */
StringList StandardPaths::standardLocations(StandardLocation type) {
    StringList result;

    TString primary = writableLocation(type);
    if(!primary.isEmpty()) {
        result.push_back(primary);
    }

#ifdef _WIN32
    if(type == AppDataLocation || type == AppLocalDataLocation) {
        TString programData = knownFolder(FOLDERID_ProgramData);
        if(!programData.isEmpty()) {
            result.push_back(programData);
        }
    } else if(type == ApplicationsLocation) {
        TString pf   = envVar(L"ProgramFiles");
        TString pf86 = envVar(L"ProgramFiles(x86)");
        if(!pf86.isEmpty() && pf86 != pf) {
            result.push_back(pf86);
        }
    }
#elif defined(__APPLE__)
    if(type == ApplicationsLocation) {
        result.push_back(TString("/Applications"));
        result.push_back(homeDir() + "/Applications");
    } else if(type == AppDataLocation || type == AppLocalDataLocation) {
        // System-wide Application Support for all users.
        result.push_back(TString("/Library/Application Support"));
    } else if(type == ConfigLocation) {
        // System-wide Preferences for all users.
        result.push_back(TString("/Library/Preferences"));
    }
#else
    switch(type) {
    case ConfigLocation: {
        const char *dirs = std::getenv("XDG_CONFIG_DIRS");
        TString list = (dirs && *dirs) ? TString(dirs) : TString("/etc/xdg");
        splitPaths(list, result);
        break;
    }
    case AppDataLocation:
    case AppLocalDataLocation: {
        const char *dirs = std::getenv("XDG_DATA_DIRS");
        TString list = (dirs && *dirs)
                           ? TString(dirs)
                           : TString("/usr/local/share:/usr/share");
        splitPaths(list, result);
        break;
    }
    case ApplicationsLocation: {
        result.push_back(TString("/usr/local/bin"));
        result.push_back(TString("/usr/bin"));
        result.push_back(TString("/bin"));
        break;
    }
    default:
        break;
    }
#endif

    result.unique();
    return result;
}

/*! Append \a organizationName and \a applicationName to \a base. */
TString StandardPaths::appendOrganizationAndApp(const TString &base, const TString &organizationName, const TString &applicationName) {
    TString result = base;
    auto append = [&result](const TString &part) {
        if(part.isEmpty()) {
            return;
        }
        if(!result.isEmpty()) {
            char last = result.back();
            if(last != '/' && last != '\\') {
#ifdef _WIN32
                result += '\\';
#else
                result += '/';
#endif
            }
        }
        result += part;
    };

    append(organizationName);
    append(applicationName);
    return result;
}
