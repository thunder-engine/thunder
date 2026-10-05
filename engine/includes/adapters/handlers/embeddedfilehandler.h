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
#ifndef EMBEDDEDFILEHANDLER_H
#define EMBEDDEDFILEHANDLER_H

#include <engine.h>
#include <file.h>

#include <algorithm>
#include <cstring>
#include <string>

class ENGINE_EXPORT EmbeddedFileHandler : public FileHandler {
public:
    struct EmbeddedFile {
        const char *name;
        const uint8_t *data;
        size_t size;
    };

    static void registerFiles(const EmbeddedFile *files, size_t count);
    static bool hasFiles();

    explicit EmbeddedFileHandler(FileHandler *fallback) :
            m_fallback(fallback) {
    }

    void mount(const char *path, bool writable = false) override {
        if(m_fallback && path && path[0] != '\0') {
            m_fallback->mount(path, writable);
        }
    }

    void unmount(const char *path) override {
        if(m_fallback && path && path[0] != '\0') {
            m_fallback->unmount(path);
        }
    }

    StringList list(const char *path, bool recursive = false) override {
        StringList result;
        if(m_fallback) {
            result = m_fallback->list(path, recursive);
        }

        std::string prefix = normalize(path);
        if(!prefix.empty()) {
            prefix += '/';
        }

        const EmbeddedFile *files = registeredFiles();
        for(size_t i = 0; i < fileCount(); ++i) {
            std::string name(files[i].name);
            if(name.compare(0, prefix.size(), prefix) != 0) {
                continue;
            }

            std::string relative = name.substr(prefix.size());
            if(relative.empty() || (!recursive && relative.find('/') != std::string::npos)) {
                continue;
            }
            TString item(relative);
            if(std::find(result.begin(), result.end(), item) == result.end()) {
                result.push_back(item);
            }
        }
        return result;
    }

    bool mkDir(const char *path) override {
        return m_fallback && m_fallback->mkDir(path);
    }

    bool mkPath(const char *path) override {
        return m_fallback && m_fallback->mkPath(path);
    }

    bool remove(const char *path) override {
        return m_fallback && m_fallback->remove(path);
    }

    bool rename(const char *origin, const char *target) override {
        return m_fallback && m_fallback->rename(origin, target);
    }

    bool copy(const char *origin, const char *target) override {
        return m_fallback && m_fallback->copy(origin, target);
    }

    bool exists(const char *path) override {
        return findFile(path) != nullptr || (m_fallback && m_fallback->exists(path));
    }

    bool isDir(const char *path) override {
        std::string prefix = normalize(path);
        if(!prefix.empty()) {
            prefix += '/';
        }
        const EmbeddedFile *files = registeredFiles();
        for(size_t i = 0; i < fileCount(); ++i) {
            if(std::string(files[i].name).compare(0, prefix.size(), prefix) == 0 &&
               std::string(files[i].name).size() > prefix.size()) {
                return true;
            }
        }
        return m_fallback && m_fallback->isDir(path);
    }

    bool isFile(const char *path) override {
        return findFile(path) != nullptr || (m_fallback && m_fallback->isFile(path));
    }

    int close(int *handle) override {
        Handle *file = reinterpret_cast<Handle *>(handle);
        int result = 0;
        if(file->fallbackHandle && m_fallback) {
            result = m_fallback->close(file->fallbackHandle);
        }
        delete file;
        return result;
    }

    int *open(const char *path, int mode) override {
        const EmbeddedFile *embedded = findFile(path);
        if(embedded) {
            if(!(mode & File::Read) || (mode & (File::Write | File::Append))) {
                return nullptr;
            }
            return reinterpret_cast<int *>(new Handle{nullptr, embedded, 0});
        }
        if(!m_fallback) {
            return nullptr;
        }
        int *handle = m_fallback->open(path, mode);
        return handle ? reinterpret_cast<int *>(new Handle{handle, nullptr, 0}) : nullptr;
    }

    size_t seek(int *handle, uint64_t origin) override {
        Handle *file = reinterpret_cast<Handle *>(handle);
        if(file->fallbackHandle) {
            return m_fallback->seek(file->fallbackHandle, origin);
        }
        if(!file->embedded || origin > file->embedded->size) {
            return 1;
        }
        file->position = static_cast<size_t>(origin);
        return 0;
    }

    size_t read(void *ptr, size_t size, size_t count, int *handle) override {
        Handle *file = reinterpret_cast<Handle *>(handle);
        if(file->fallbackHandle) {
            return m_fallback->read(ptr, size, count, file->fallbackHandle);
        }
        if(!file->embedded || size == 0) {
            return 0;
        }
        size_t available = file->embedded->size - file->position;
        size_t items = std::min(count, available / size);
        if(items == 0) {
            return 0;
        }
        std::memcpy(ptr, file->embedded->data + file->position, items * size);
        file->position += items * size;
        return items;
    }

    size_t write(const void *ptr, size_t size, size_t count, int *handle) override {
        Handle *file = reinterpret_cast<Handle *>(handle);
        return file->fallbackHandle && m_fallback ?
                    m_fallback->write(ptr, size, count, file->fallbackHandle) : 0;
    }

    size_t size(int *handle) override {
        Handle *file = reinterpret_cast<Handle *>(handle);
        return file->fallbackHandle && m_fallback ?
                    m_fallback->size(file->fallbackHandle) :
                    (file->embedded ? file->embedded->size : 0);
    }

    size_t tell(int *handle) override {
        Handle *file = reinterpret_cast<Handle *>(handle);
        return file->fallbackHandle && m_fallback ?
                    m_fallback->tell(file->fallbackHandle) : file->position;
    }

private:
    struct Handle {
        int *fallbackHandle;
        const EmbeddedFile *embedded;
        size_t position;
    };

    static std::string normalize(const char *path) {
        std::string result(path ? path : "");
        std::replace(result.begin(), result.end(), '\\', '/');
        while(!result.empty() && result.front() == '/') {
            result.erase(result.begin());
        }
        while(result.compare(0, 2, "./") == 0) {
            result.erase(0, 2);
        }
        return result;
    }

    static const EmbeddedFile *findFile(const char *path) {
        std::string name = normalize(path);
        for(size_t i = 0; i < fileCount(); ++i) {
            const EmbeddedFile *files = registeredFiles();
            if(name == files[i].name) {
                return &files[i];
            }
        }
        return nullptr;
    }

private:
    static const EmbeddedFile *registeredFiles();
    static size_t fileCount();

    FileHandler *m_fallback;
};

#endif // EMBEDDEDFILEHANDLER_H
