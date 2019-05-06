#include "file.h"

#include "log.h"

#include <physfs.h>

void IFile::finit(const char *argv0) {
    if(!PHYSFS_init(argv0)) {
        Log(Log::ERR) << "[ FileIO ] Can't initialize.";
    }
}

void IFile::fsearchPathAdd(const char *path, bool isFirst) {
    if(PHYSFS_addToSearchPath(path, isFirst ? 0 : 1) == 0) {
        Log(Log::ERR) << "[ FileIO ] Filed to add search path." << path << PHYSFS_getLastError();
    }
    if(isFirst && PHYSFS_setWriteDir(path) == 0) {
        Log(Log::ERR) << "[ FileIO ] Can't set directory for writing.";
    }
}

StringList IFile::_flist(const char *path) {
    char **rc = PHYSFS_enumerateFiles(path);
    char **i;

    StringList result;
    for(i = rc; *i != nullptr; i++) {
        result.push_back(*i);
    }

    PHYSFS_freeList(rc);

    return result;
}

bool IFile::_mkdir(const char *path) {
    return (PHYSFS_mkdir(path) == 0);
}

bool IFile::_delete(const char *path) {
    bool result = (PHYSFS_delete(path) != 0);
    if(!result) {
        Log(Log::ERR) << "[ FileIO ] Can't delete file" << path;
    }
    return result;
}

bool IFile::_exists(const char *path) {
    return PHYSFS_exists(path);
}

bool IFile::_isdir(const char *path) {
    return PHYSFS_isDirectory(path);
}

int IFile::_fclose(_FILE *stream) {
    return PHYSFS_close(static_cast<PHYSFS_file *>(stream));
}

_size_t IFile::_fseek(_FILE *stream, uint64_t offset, int origin) {
    A_UNUSED(origin)
    return static_cast<_size_t>(PHYSFS_seek(static_cast<PHYSFS_file *>(stream), offset));
}

_FILE *IFile::_fopen(const char *path, const char *mode) {
    _FILE *result = nullptr;
    switch (mode[0]) {
        case 'r': result = static_cast<void *>(PHYSFS_openRead(path)); break;
        case 'w': result = static_cast<void *>(PHYSFS_openWrite(path)); break;
        case 'a': result = static_cast<void *>(PHYSFS_openAppend(path)); break;
        default: break;
    }
    if(result == nullptr) {
        Log(Log::ERR) << "[ FileIO ] Can't open file" << path;
    }
    return result;
}

_size_t IFile::_fread(void *ptr, _size_t size, _size_t count, _FILE *stream) {
    return static_cast<_size_t>(PHYSFS_read(static_cast<PHYSFS_file *>(stream), ptr, size, count));
}

_size_t IFile::_fwrite(const void *ptr, _size_t size, _size_t count, _FILE *stream) {
    return static_cast<_size_t>(PHYSFS_write(static_cast<PHYSFS_file *>(stream), ptr, size, count));
}

_size_t IFile::_fsize(_FILE *stream) {
    return static_cast<_size_t>(PHYSFS_fileLength(static_cast<PHYSFS_file *>(stream)));
}

_size_t IFile::_ftell(_FILE *stream) {
    return static_cast<_size_t>(PHYSFS_tell(static_cast<PHYSFS_file *>(stream)));
}

const char *IFile::baseDir() const {
    return PHYSFS_getBaseDir();
}

const char *IFile::userDir() const {
    return PHYSFS_getUserDir();
}

