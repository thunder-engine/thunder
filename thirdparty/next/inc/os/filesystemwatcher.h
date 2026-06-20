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

#ifndef FILESYSTEMWATCHER_H
#define FILESYSTEMWATCHER_H

#include <objectsystem.h>

class FileSystemWatcherPrivate;

class NEXT_LIBRARY_EXPORT FileSystemWatcher : public Object {
    A_OBJECT(FileSystemWatcher, Object, Core)

    A_METHODS(
        A_SIGNAL(FileSystemWatcher::fileChanged),
        A_SIGNAL(FileSystemWatcher::directoryChanged)
    )

public:
    FileSystemWatcher();
    ~FileSystemWatcher();

    bool addPath(const TString &path);
    bool addPaths(const StringList &paths);

    bool removePath(const TString &path);
    bool removePaths(const StringList &paths);

    StringList directories() const;
    StringList files() const;

public: // signals
    void fileChanged(const TString &path);

    void directoryChanged(const TString &path);

private:
    bool event(Event *event) override;

private:
    FileSystemWatcherPrivate *m_ptr;

};

#endif // FILESYSTEMWATCHER_H
