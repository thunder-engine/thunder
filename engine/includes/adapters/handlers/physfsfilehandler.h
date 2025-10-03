#ifndef PHYSFSFILEHANDLER_H
#define PHYSFSFILEHANDLER_H

#include <physfs.h>

#include <file.h>
#include <log.h>

#include "engine.h"
#include "systems/resourcesystem.h"

class PhysfsFileHandler : public FileHandler {
public:
    void init(const char *argv0) {
        if(!PHYSFS_init(argv0)) {
            aError() << "[ FileIO ] Can't initialize.";
        }
    }

    void searchPathAdd(const char *path, bool writable = false) {
        if(PHYSFS_addToSearchPath(path, writable ? 0 : 1) == 0) {
            aError() << "[ FileIO ] Filed to add search path." << path << PHYSFS_getLastError();
        }
        if(writable && PHYSFS_setWriteDir(path) == 0) {
            aError() << "[ FileIO ] Can't set directory for writing.";
        }
    }

protected:
    StringList list(const char *path) override {
        char **rc = PHYSFS_enumerateFiles(path);
        char **i;

        StringList result;
        for(i = rc; *i != nullptr; i++) {
            result.push_back(*i);
        }

        PHYSFS_freeList(rc);

        return result;
    }

    bool mkDir(const char *path) override {
        return (PHYSFS_mkdir(path) == 0);
    }

    bool remove(const char *path) override {
        bool result = (PHYSFS_delete(path) != 0);
        if(!result) {
            aError() << "[ FileIO ] Can't delete file" << path;
        }
        return result;
    }

    void rename(const char *, const char *) override {

    }

    void copy(const char *, const char *) override {

    }

    bool exists(const char *path) override {
        return PHYSFS_exists(path);
    }

    bool isDir(const char *path) override {
        return PHYSFS_isDirectory(path);
    }

    bool isFile(const char *path) override {
        return !PHYSFS_isDirectory(path) && !PHYSFS_isSymbolicLink(path);
    }

    int close(int *handle) override {
        return PHYSFS_close(reinterpret_cast<PHYSFS_file *>(handle));
    }

    size_t seek(int *handle, uint64_t origin) override {
        A_UNUSED(origin);
        return static_cast<size_t>(PHYSFS_seek(reinterpret_cast<PHYSFS_file *>(handle), origin));
    }

    int *open(const char *path, int mode) override {
        if(mode & File::Append) {
            return reinterpret_cast<int *>(PHYSFS_openAppend(path));
        }

        if(mode & File::ReadOnly) {
            return reinterpret_cast<int *>(PHYSFS_openRead(path));
        }

        if(mode & File::WriteOnly) {
            return reinterpret_cast<int *>(PHYSFS_openWrite(path));
        }

        aWarning() << "[ FileIO ] Can't open file" << path;

        return nullptr;
    }

    size_t read(void *ptr, size_t size, size_t count, int *handle) override {
        return static_cast<size_t>(PHYSFS_read(reinterpret_cast<PHYSFS_file *>(handle), ptr, size, count));
    }

    size_t write(const void *ptr, size_t size, size_t count, int *handle) override {
        return static_cast<size_t>(PHYSFS_write(reinterpret_cast<PHYSFS_file *>(handle), ptr, size, count));
    }

    size_t size(int *handle) override {
        return static_cast<size_t>(PHYSFS_fileLength(reinterpret_cast<PHYSFS_file *>(handle)));
    }

    size_t tell(int *handle) override {
        return static_cast<size_t>(PHYSFS_tell(reinterpret_cast<PHYSFS_file *>(handle)));
    }

    TString md5(const char *path) override {
        const ResourceSystem::Dictionary &indices = Engine::resourceSystem()->indices();
        auto it = indices.find(path);
        if(it != indices.end()) {
            return it->second.md5;
        }

        return TString();
    }
};

#endif // PHYSFSFILEHANDLER_H
