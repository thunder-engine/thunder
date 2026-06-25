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

#ifndef FILEDIALOG_H
#define FILEDIALOG_H

#include <astring.h>
#include <memory>

class FileDialogPrivate;

class NEXT_LIBRARY_EXPORT FileDialog {
public:
    enum Mode {
        OpenFile,
        SaveFile,
        OpenDirectory,
        OpenFiles
    };

    FileDialog();
    ~FileDialog();

    FileDialog(const FileDialog&) = delete;
    FileDialog &operator=(const FileDialog&) = delete;

    FileDialog(FileDialog&&) noexcept;
    FileDialog &operator=(FileDialog&&) noexcept;

    void setMode(Mode m);
    void setShowHidden(bool show);
    void setDefaultSuffix(const TString &suffix);
    void setWindowTitle(const TString &title);

    void addFilter(const TString &name, const StringList &extensions);
    void clearFilters();

    void setDirectory(const TString &dir);

    TString getSelectedFile() const;
    StringList getSelectedFiles() const;

    bool exec();

private:
    FileDialogPrivate *m_ptr;

};

#endif // FILEDIALOG_H
