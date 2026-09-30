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

#include "os/library.h"

#include <string>
#include <cstring>

class LibraryPrivate {
public:
    LibraryPrivate() :
        m_hints(0),
        m_handle(nullptr),
        m_loaded(false) {
    }

    ~LibraryPrivate() {
        if(m_loaded) {
            unload();
        }
    }

    bool load();
    bool unload();
    void *resolve(const char *symbol);

private:
    friend class Library;

#ifdef _WIN32
    bool loadWindows();
    bool unloadWindows();
    void *resolveWindows(const char *symbol);
    TString errorStringWindows() const;
#endif

#ifdef __APPLE__
    bool loadApple();
    bool unloadApple();
    void *resolveApple(const char *symbol);
    TString errorStringApple() const;
#endif

#ifdef __linux__
    bool loadLinux();
    bool unloadLinux();
    void *resolveLinux(const char *symbol);
    TString errorStringLinux() const;
#endif

    TString m_fileName;
    int m_hints;

    void *m_handle;
    bool m_loaded;
    mutable TString m_errorString;
};

bool LibraryPrivate::load() {
    if(m_loaded) {
        return true;
    }

    if(m_fileName.isEmpty()) {
        m_errorString = "Library file name is empty";
        return false;
    }

    m_errorString.clear();

#ifdef _WIN32
    return loadWindows();
#elif __APPLE__
    return loadApple();
#elif __linux__
    return loadLinux();
#else
#error "Unsupported platform"
#endif
}

bool LibraryPrivate::unload() {
    if(!m_loaded) {
        return true;
    }

#ifdef _WIN32
    return unloadWindows();
#elif __APPLE__
    return unloadApple();
#elif __linux__
    return unloadLinux();
#else
#error "Unsupported platform"
#endif
}

void *LibraryPrivate::resolve(const char *symbol) {
    if(!m_loaded || symbol == nullptr) {
        return nullptr;
    }

    m_errorString.clear();

#ifdef _WIN32
    return resolveWindows(symbol);
#elif __APPLE__
    return resolveApple(symbol);
#elif __linux__
    return resolveLinux(symbol);
#else
#error "Unsupported platform"
#endif
}

#ifdef _WIN32
#include <windows.h>

bool LibraryPrivate::loadWindows() {
    HMODULE handle = LoadLibraryExA(m_fileName.data(), nullptr,
                                    LOAD_WITH_ALTERED_SEARCH_PATH);
    if(handle == nullptr) {
        m_errorString = errorStringWindows();
        return false;
    }

    m_handle = handle;
    m_loaded = true;
    return true;
}

bool LibraryPrivate::unloadWindows() {
    if(m_hints & Library::PreventUnloadHint) {
        m_handle = nullptr;
        m_loaded = false;
        return true;
    }

    if(m_handle != nullptr) {
        if(!FreeLibrary(static_cast<HMODULE>(m_handle))) {
            m_errorString = errorStringWindows();
            return false;
        }
    }

    m_handle = nullptr;
    m_loaded = false;
    return true;
}

void *LibraryPrivate::resolveWindows(const char *symbol) {
    if(m_handle == nullptr) {
        return nullptr;
    }

    FARPROC proc = GetProcAddress(static_cast<HMODULE>(m_handle), symbol);
    if(proc == nullptr) {
        m_errorString = errorStringWindows();
        return nullptr;
    }

    return reinterpret_cast<void *>(proc);
}

TString LibraryPrivate::errorStringWindows() const {
    DWORD errorCode = GetLastError();
    if(errorCode == 0) {
        return TString();
    }

    LPSTR buffer = nullptr;
    DWORD size = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        errorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPSTR>(&buffer),
        0,
        nullptr);

    TString result;
    if(size > 0 && buffer != nullptr) {
        while(size > 0 && (buffer[size - 1] == '\r' || buffer[size - 1] == '\n')) {
            --size;
        }
        result = TString(std::string(buffer, size));
    }

    if(buffer != nullptr) {
        LocalFree(buffer);
    }

    return result;
}
#endif // _WIN32

#ifdef __APPLE__
#include <dlfcn.h>

bool LibraryPrivate::loadApple() {
    int flags = RTLD_LAZY | RTLD_LOCAL;
    if(m_hints & Library::ResolveAllSymbolsHint) {
        flags &= ~RTLD_LAZY;
        flags |= RTLD_NOW;
    }
    if(m_hints & Library::ExportExternalSymbolsHint) {
        flags &= ~RTLD_LOCAL;
        flags |= RTLD_GLOBAL;
    }

    m_handle = dlopen(m_fileName.data(), flags);
    if(m_handle == nullptr) {
        m_errorString = errorStringApple();
        return false;
    }

    m_loaded = true;
    return true;
}

bool LibraryPrivate::unloadApple() {
    if(m_hints & Library::PreventUnloadHint) {
        m_handle = nullptr;
        m_loaded = false;
        return true;
    }

    if(m_handle != nullptr) {
        if(dlclose(m_handle) != 0) {
            m_errorString = errorStringApple();
            return false;
        }
    }

    m_handle = nullptr;
    m_loaded = false;
    return true;
}

void *LibraryPrivate::resolveApple(const char *symbol) {
    if(m_handle == nullptr) {
        return nullptr;
    }

    dlerror(); // clear previous error

    void *proc = dlsym(m_handle, symbol);
    if(proc == nullptr) {
        m_errorString = errorStringApple();
        return nullptr;
    }

    return proc;
}

TString LibraryPrivate::errorStringApple() const {
    const char *err = dlerror();
    if(err == nullptr) {
        return TString();
    }
    return TString(err);
}
#endif // __APPLE__

#ifdef __linux__
#include <dlfcn.h>

bool LibraryPrivate::loadLinux() {
    int flags = RTLD_LAZY | RTLD_LOCAL;
    if(m_hints & Library::ResolveAllSymbolsHint) {
        flags &= ~RTLD_LAZY;
        flags |= RTLD_NOW;
    }
    if(m_hints & Library::ExportExternalSymbolsHint) {
        flags &= ~RTLD_LOCAL;
        flags |= RTLD_GLOBAL;
    }

    m_handle = dlopen(m_fileName.data(), flags);
    if(m_handle == nullptr) {
        m_errorString = errorStringLinux();
        return false;
    }

    m_loaded = true;
    return true;
}

bool LibraryPrivate::unloadLinux() {
    if(m_hints & Library::PreventUnloadHint) {
        m_handle = nullptr;
        m_loaded = false;
        return true;
    }

    if(m_handle != nullptr) {
        if(dlclose(m_handle) != 0) {
            m_errorString = errorStringLinux();
            return false;
        }
    }

    m_handle = nullptr;
    m_loaded = false;
    return true;
}

void *LibraryPrivate::resolveLinux(const char *symbol) {
    if(m_handle == nullptr) {
        return nullptr;
    }

    dlerror(); // clear previous error

    void *proc = dlsym(m_handle, symbol);
    if(proc == nullptr) {
        m_errorString = errorStringLinux();
        return nullptr;
    }

    return proc;
}

TString LibraryPrivate::errorStringLinux() const {
    const char *err = dlerror();
    if(err == nullptr) {
        return TString();
    }
    return TString(err);
}
#endif // __linux__

/*!
    \class Library
    \brief Platform-independent wrapper around shared library loading.
    \since Next 1.0
    \inmodule OS

    `Library` provides a portable interface for loading shared libraries
    (DLL on Windows, dylib on macOS, so on Linux), resolving exported
    symbols by name, and unloading libraries when they are no longer
    needed.

    The class is non-copyable but movable, so it can be stored in
    standard containers. When a `Library` instance is destroyed and the
    library is still loaded, it is unloaded automatically unless the
    \l{LoadHint}{PreventUnloadHint} flag was set.

    Typical usage:

    \code
    Library lib("plugin");
    if(lib.load()) {
        typedef void (*InitFunc)();
        InitFunc init = reinterpret_cast<InitFunc>(lib.resolve("plugin_init"));
        if(init) {
            init();
        }
    } else {
        aError() << lib.errorString();
    }
    \endcode

    \sa LoadHint, load(), resolve()
*/

/*!
    \enum Library::LoadHint

    Flags that control how the shared library is loaded.

    \value ResolveAllSymbolsHint
           Resolve all symbols immediately at load time (RTLD_NOW on
           Linux/macOS). On Windows this hint is ignored, because the
           platform always resolves imports eagerly.

    \value ExportExternalSymbolsHint
           Make the library's symbols available for subsequently loaded
           libraries (RTLD_GLOBAL on Linux/macOS). On Windows this hint
           is ignored.

    \value PreventUnloadHint
           Prevent the library from being unloaded, even if unload()
           is called or the Library instance is destroyed. Useful when
           external code holds pointers into the library's memory.
*/

/*!
    Constructs a `Library` object associated with the shared library
    named \a fileName.

    The library is not loaded until load() is called. If \a fileName is
    empty, the object is constructed in an "unbound" state; a name can
    be assigned later via setFileName().

    \sa setFileName(), load()
*/
Library::Library(const TString &fileName) :
    m_ptr(new LibraryPrivate) {
    m_ptr->m_fileName = fileName;
}

Library::~Library() {
    delete m_ptr;
}

/*!
    Move-constructs a `Library` from \a other. After the move, \a other
    is left in a valid but unspecified state (typically unloaded).
*/
Library::Library(Library&&) noexcept = default;

/*!
    Move-assigns \a other to this `Library`. If this object owns a loaded
    library, it is unloaded first (unless `PreventUnloadHint` is set).
*/
Library& Library::operator=(Library&&) noexcept = default;

/*!
    Sets the file name of the shared library to \a fileName.

    This function has no effect if the library is already loaded;
    call unload() first.

    \sa fileName(), load()
*/
void Library::setFileName(const TString &fileName) {
    if(m_ptr->m_loaded) {
        return;
    }
    m_ptr->m_fileName = fileName;
}

/*!
    Returns the file name of the shared library.

    \sa setFileName()
*/
TString Library::fileName() const {
    return m_ptr->m_fileName;
}

/*!
    Sets the load hints to \a hints.

    The \a hints parameter is a bitwise OR of the \l{LoadHint} values.
    This function has no effect if the library is already loaded;
    call unload() first.

    \sa loadHints(), LoadHint
*/
void Library::setLoadHints(int hints) {
    if(m_ptr->m_loaded) {
        return;
    }
    m_ptr->m_hints = hints;
}

/*!
    Returns the current load hints.

    \sa setLoadHints()
*/
int Library::loadHints() const {
    return m_ptr->m_hints;
}

/*!
    Loads the shared library into the process address space.

    Returns true on success; otherwise returns false. If the library
    is already loaded, the function returns true immediately.

    If the file name was not set, or the library cannot be loaded,
    errorString() can be used to obtain a human-readable description
    of the failure.

    \sa unload(), isLoaded(), errorString()
*/
bool Library::load() {
    return m_ptr->load();
}

/*!
    Unloads the shared library.

    Returns true on success; otherwise returns false. If the library
    is not loaded, the function returns true immediately.

    If the \l{LoadHint}{PreventUnloadHint} flag was set, the library
    is not actually unloaded; the internal handle is simply released
    and the object transitions to the unloaded state.

    \sa load(), isLoaded()
*/
bool Library::unload() {
    return m_ptr->unload();
}

/*!
    Returns true if the library is currently loaded; otherwise returns
    false.

    \sa load(), unload()
*/
bool Library::isLoaded() const {
    return m_ptr->m_loaded;
}

/*!
    Returns the address of the exported symbol \a symbol, or `nullptr`
    if the symbol cannot be found.

    The returned pointer is a raw function or data address and must
    be cast to the appropriate function-pointer type by the caller.
    If the library is not loaded or \a symbol is `nullptr`, the
    function returns `nullptr` without modifying the error string.

    \sa load(), errorString()
*/
void *Library::resolve(const char *symbol) {
    return m_ptr->resolve(symbol);
}

/*!
    Returns a human-readable description of the last error that
    occurred in load() or resolve().

    If no error has occurred, an empty string is returned.

    \sa load(), resolve()
*/
TString Library::errorString() const {
    return m_ptr->m_errorString;
}

/*!
    Returns true if \a fileName has the shared-library extension for
    the current platform; otherwise returns false.

    The comparison is case-insensitive:

    \list
        \li Windows: `.dll`
        \li macOS:   `.dylib` or `.so`
        \li Linux:   `.so`
    \endlist

    \sa load()
*/
bool Library::isLibrary(const TString &fileName) {
    if(fileName.isEmpty()) {
        return false;
    }

    const std::string lower = fileName.toLower().toStdString();

    auto endsWith = [&lower](const char *ext) {
        const size_t extLen = std::strlen(ext);
        if(lower.size() < extLen) {
            return false;
        }
        return lower.compare(lower.size() - extLen, extLen, ext) == 0;
    };

#ifdef _WIN32
    return endsWith(".dll");
#elif __APPLE__
    return endsWith(".dylib") || endsWith(".so");
#elif __linux__
    return endsWith(".so");
#else
    return false;
#endif
}
