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

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

class EmbeddedFileHandler : public FileHandler {
public:
    struct EmbeddedFile {
        const char *name;
        const uint8_t *data;
        size_t size;
    };

    static uint64_t registerFiles(const EmbeddedFile *files, size_t count);
    static void unregisterFiles(uint64_t registrationId);
    static bool hasFiles();

    explicit EmbeddedFileHandler(FileHandler *fallback);

    void mount(const char *path, bool writable = false) override;
    void unmount(const char *path) override;
    StringList list(const char *path, bool recursive = false) override;
    bool mkDir(const char *path) override;
    bool mkPath(const char *path) override;
    bool remove(const char *path) override;
    bool rename(const char *origin, const char *target) override;
    bool copy(const char *origin, const char *target) override;
    bool exists(const char *path) override;
    bool isDir(const char *path) override;
    bool isFile(const char *path) override;
    int close(int *handle) override;
    int *open(const char *path, int mode) override;
    size_t seek(int *handle, uint64_t origin) override;
    size_t read(void *ptr, size_t size, size_t count, int *handle) override;
    size_t write(const void *ptr, size_t size, size_t count, int *handle) override;
    size_t size(int *handle) override;
    size_t tell(int *handle) override;

private:
    struct Handle;

    static std::string normalize(const char *path);
    static bool findFile(const char *path, uint64_t &registrationId, size_t &fileIndex);
    static std::vector<std::string> registeredNames();
    static bool getResourceSize(uint64_t registrationId, size_t fileIndex, size_t &size);
    static size_t readResource(uint64_t registrationId, size_t fileIndex, void *ptr,
                               size_t size, size_t count, size_t &position);

    FileHandler *m_fallback;
};

class ENGINE_EXPORT EmbeddedFileRegistration {
public:
    EmbeddedFileRegistration(const EmbeddedFileHandler::EmbeddedFile *files, size_t count);
    ~EmbeddedFileRegistration();

    EmbeddedFileRegistration(const EmbeddedFileRegistration &) = delete;
    EmbeddedFileRegistration &operator=(const EmbeddedFileRegistration &) = delete;

private:
    uint64_t m_registrationId;
};

#endif // EMBEDDEDFILEHANDLER_H
