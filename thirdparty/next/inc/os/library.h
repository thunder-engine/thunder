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

#ifndef LIBRARY_H
#define LIBRARY_H

#include <astring.h>

class LibraryPrivate;

class NEXT_LIBRARY_EXPORT Library {
public:
    enum LoadHint {
        ResolveAllSymbolsHint     = 0x01,
        ExportExternalSymbolsHint = 0x02,
        PreventUnloadHint         = 0x04
    };

    explicit Library(const TString &fileName = TString());
    ~Library();

    Library(const Library&) = delete;
    Library &operator=(const Library&) = delete;

    Library(Library&&) noexcept;
    Library &operator=(Library&&) noexcept;

    void setFileName(const TString &fileName);
    TString fileName() const;

    void setLoadHints(int hints);
    int loadHints() const;

    bool load();
    bool unload();
    bool isLoaded() const;

    void *resolve(const char *symbol);

    TString errorString() const;

    // Static check: is the given file name a shared library for this platform
    static bool isLibrary(const TString &fileName);

private:
    LibraryPrivate *m_ptr;
};

#endif // LIBRARY_H
