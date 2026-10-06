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
#include "adapters/handlers/embeddedfilehandler.h"

#include <algorithm>
#include <mutex>
#include <vector>

struct EmbeddedFileHandler::Handle {
    int *fallbackHandle;
    uint64_t registrationId;
    size_t fileIndex;
    size_t position;
};

namespace {
    struct RegisteredTable {
        uint64_t id;
        const EmbeddedFileHandler::EmbeddedFile *files;
        size_t count;
    };

    struct ResourceRegistry {
        std::mutex mutex;
        std::vector<RegisteredTable> tables;
        uint64_t nextId = 1;
    };

    ResourceRegistry &registry() {
        static ResourceRegistry *instance = new ResourceRegistry;
        return *instance;
    }
}

EmbeddedFileHandler::EmbeddedFileHandler(FileHandler *fallback) :
        m_fallback(fallback) {
}

void EmbeddedFileHandler::mount(const char *path, bool writable) {
    if(m_fallback && path && path[0] != '\0') {
        m_fallback->mount(path, writable);
    }
}

void EmbeddedFileHandler::unmount(const char *path) {
    if(m_fallback && path && path[0] != '\0') {
        m_fallback->unmount(path);
    }
}

StringList EmbeddedFileHandler::list(const char *path, bool recursive) {
    StringList result;
    if(m_fallback) {
        result = m_fallback->list(path, recursive);
    }

    std::string prefix = normalize(path);
    if(!prefix.empty()) {
        prefix += '/';
    }

    for(const std::string &name : registeredNames()) {
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

bool EmbeddedFileHandler::mkDir(const char *path) {
    return m_fallback && m_fallback->mkDir(path);
}

bool EmbeddedFileHandler::mkPath(const char *path) {
    return m_fallback && m_fallback->mkPath(path);
}

bool EmbeddedFileHandler::remove(const char *path) {
    return m_fallback && m_fallback->remove(path);
}

bool EmbeddedFileHandler::rename(const char *origin, const char *target) {
    return m_fallback && m_fallback->rename(origin, target);
}

bool EmbeddedFileHandler::copy(const char *origin, const char *target) {
    return m_fallback && m_fallback->copy(origin, target);
}

bool EmbeddedFileHandler::exists(const char *path) {
    uint64_t registrationId = 0;
    size_t fileIndex = 0;
    return findFile(path, registrationId, fileIndex) || (m_fallback && m_fallback->exists(path));
}

bool EmbeddedFileHandler::isDir(const char *path) {
    std::string prefix = normalize(path);
    if(!prefix.empty()) {
        prefix += '/';
    }
    for(const std::string &name : registeredNames()) {
        if(name.compare(0, prefix.size(), prefix) == 0 &&
           name.size() > prefix.size()) {
            return true;
        }
    }
    return m_fallback && m_fallback->isDir(path);
}

bool EmbeddedFileHandler::isFile(const char *path) {
    uint64_t registrationId = 0;
    size_t fileIndex = 0;
    return findFile(path, registrationId, fileIndex) || (m_fallback && m_fallback->isFile(path));
}

int EmbeddedFileHandler::close(int *handle) {
    Handle *file = reinterpret_cast<Handle *>(handle);
    int result = 0;
    if(file->fallbackHandle && m_fallback) {
        result = m_fallback->close(file->fallbackHandle);
    }
    delete file;
    return result;
}

int *EmbeddedFileHandler::open(const char *path, int mode) {
    uint64_t registrationId = 0;
    size_t fileIndex = 0;
    if(findFile(path, registrationId, fileIndex)) {
        if(!(mode & File::Read) || (mode & (File::Write | File::Append))) {
            return nullptr;
        }
        return reinterpret_cast<int *>(new Handle{nullptr, registrationId, fileIndex, 0});
    }
    if(!m_fallback) {
        return nullptr;
    }
    int *handle = m_fallback->open(path, mode);
    return handle ? reinterpret_cast<int *>(new Handle{handle, 0, 0, 0}) : nullptr;
}

size_t EmbeddedFileHandler::seek(int *handle, uint64_t origin) {
    Handle *file = reinterpret_cast<Handle *>(handle);
    if(file->fallbackHandle) {
        return m_fallback->seek(file->fallbackHandle, origin);
    }
    size_t resourceSize = 0;
    if(file->registrationId == 0 ||
       !getResourceSize(file->registrationId, file->fileIndex, resourceSize) ||
       origin > resourceSize) {
        return 1;
    }
    file->position = static_cast<size_t>(origin);
    return 0;
}

size_t EmbeddedFileHandler::read(void *ptr, size_t size, size_t count, int *handle) {
    Handle *file = reinterpret_cast<Handle *>(handle);
    if(file->fallbackHandle) {
        return m_fallback->read(ptr, size, count, file->fallbackHandle);
    }
    if(file->registrationId == 0 || size == 0) {
        return 0;
    }
    return readResource(file->registrationId, file->fileIndex, ptr, size, count, file->position);
}

size_t EmbeddedFileHandler::write(const void *ptr, size_t size, size_t count, int *handle) {
    Handle *file = reinterpret_cast<Handle *>(handle);
    return file->fallbackHandle && m_fallback ?
                m_fallback->write(ptr, size, count, file->fallbackHandle) : 0;
}

size_t EmbeddedFileHandler::size(int *handle) {
    Handle *file = reinterpret_cast<Handle *>(handle);
    if(file->fallbackHandle && m_fallback) {
        return m_fallback->size(file->fallbackHandle);
    }
    size_t resourceSize = 0;
    return getResourceSize(file->registrationId, file->fileIndex, resourceSize) ?
                resourceSize : 0;
}

size_t EmbeddedFileHandler::tell(int *handle) {
    Handle *file = reinterpret_cast<Handle *>(handle);
    return file->fallbackHandle && m_fallback ?
                m_fallback->tell(file->fallbackHandle) : file->position;
}

std::string EmbeddedFileHandler::normalize(const char *path) {
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

uint64_t EmbeddedFileHandler::registerFiles(const EmbeddedFile *files, size_t count) {
    if(files == nullptr || count == 0) {
        return 0;
    }

    ResourceRegistry &resources = registry();
    std::lock_guard<std::mutex> lock(resources.mutex);
    uint64_t id = resources.nextId++;
    if(id == 0) {
        id = resources.nextId++;
    }
    resources.tables.push_back({id, files, count});
    return id;
}

void EmbeddedFileHandler::unregisterFiles(uint64_t registrationId) {
    if(registrationId == 0) {
        return;
    }

    ResourceRegistry &resources = registry();
    std::lock_guard<std::mutex> lock(resources.mutex);
    for(auto it = resources.tables.begin(); it != resources.tables.end(); ++it) {
        if(it->id == registrationId) {
            resources.tables.erase(it);
            return;
        }
    }
}

bool EmbeddedFileHandler::hasFiles() {
    ResourceRegistry &resources = registry();
    std::lock_guard<std::mutex> lock(resources.mutex);
    return !resources.tables.empty();
}

std::vector<std::string> EmbeddedFileHandler::registeredNames() {
    std::vector<std::string> names;
    ResourceRegistry &resources = registry();
    std::lock_guard<std::mutex> lock(resources.mutex);
    for(auto table = resources.tables.rbegin(); table != resources.tables.rend(); ++table) {
        for(size_t i = 0; i < table->count; ++i) {
            if(table->files[i].name) {
                names.emplace_back(table->files[i].name);
            }
        }
    }
    return names;
}

bool EmbeddedFileHandler::findFile(const char *path, uint64_t &registrationId, size_t &fileIndex) {
    const std::string name = normalize(path);
    ResourceRegistry &resources = registry();
    std::lock_guard<std::mutex> lock(resources.mutex);
    for(auto table = resources.tables.rbegin(); table != resources.tables.rend(); ++table) {
        for(size_t i = 0; i < table->count; ++i) {
            if(table->files[i].name && name == table->files[i].name) {
                registrationId = table->id;
                fileIndex = i;
                return true;
            }
        }
    }
    return false;
}

bool EmbeddedFileHandler::getResourceSize(uint64_t registrationId, size_t fileIndex, size_t &size) {
    ResourceRegistry &resources = registry();
    std::lock_guard<std::mutex> lock(resources.mutex);
    for(const RegisteredTable &table : resources.tables) {
        if(table.id == registrationId && fileIndex < table.count) {
            size = table.files[fileIndex].size;
            return true;
        }
    }
    return false;
}

size_t EmbeddedFileHandler::readResource(uint64_t registrationId, size_t fileIndex, void *ptr,
                                         size_t size, size_t count, size_t &position) {
    if(size == 0) {
        return 0;
    }

    ResourceRegistry &resources = registry();
    std::lock_guard<std::mutex> lock(resources.mutex);
    for(const RegisteredTable &table : resources.tables) {
        if(table.id == registrationId && fileIndex < table.count) {
            const EmbeddedFile &file = table.files[fileIndex];
            if(position > file.size) {
                return 0;
            }
            const size_t available = file.size - position;
            const size_t items = std::min(count, available / size);
            if(items > 0) {
                std::memcpy(ptr, file.data + position, items * size);
                position += items * size;
            }
            return items;
        }
    }
    return 0;
}

EmbeddedFileRegistration::EmbeddedFileRegistration(
        const EmbeddedFileHandler::EmbeddedFile *files, size_t count) :
        m_registrationId(EmbeddedFileHandler::registerFiles(files, count)) {
}

EmbeddedFileRegistration::~EmbeddedFileRegistration() {
    EmbeddedFileHandler::unregisterFiles(m_registrationId);
}
