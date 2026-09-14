/*
    This file is part of Thunder Engine.

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
#ifndef BASEASSETPROVIDER_H
#define BASEASSETPROVIDER_H

#include <editor.h>

class FileSystemWatcher;

class EDITOR_EXPORT BaseAssetProvider : public Object {
    A_OBJECT(BaseAssetProvider, Object, Core)

    A_METHODS(
        A_SLOT(BaseAssetProvider::onFileChanged),
        A_SLOT(BaseAssetProvider::onDirectoryChanged)
    )

public:
    BaseAssetProvider();
    ~BaseAssetProvider();

    void init(bool force);

    void renameResource(const TString &source, const TString &destination);
    void removeResource(const TString &source);
    void duplicateResource(const TString &source);

public: // slots
    void onFileChanged(const TString &path);
    void onFileChangedForce(const TString &path, bool force = false);

    void onDirectoryChanged(const TString &path);
    void onDirectoryChangedForce(const TString &path, bool force = false, bool watch = false);

private:
    FileSystemWatcher *m_dirWatcher;

};

#endif // BASEASSETPROVIDER_H
