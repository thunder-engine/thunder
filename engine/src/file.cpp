#include "file.h"

#include "log.h"

#include <physfs.h>

/*!
    \class File
    \brief Basic file system I/O module.
    \inmodule Engine

    The File class provides an interface for reading from and writing to files.
    File expects the file separator to be '/' regardless of operating system. The use of other separators (e.g., '\') is not supported.

    You can check for a file's existence using exists(), and remove a file using delete().
    You can create a directory using mkdir(), list all files in directory using flist() and retrive other basic information.

    The file can be opened with open() and closed with fclose(). Data is usually can be read with fread() and written with fwrite().

    Common usecase:
    \code
    File *file = Engine::file();
    _FILE *fp = file->fopen("filename", "r");
    if(fp) {
        ByteArray data;
        data.resize(file->fsize(fp));
        file->fread(&data[0], data.size(), 1, fp);
        file->fclose(fp);
    }
    \endcode
*/

/*!
    Initialize the file system module at \a argv0 application file path.
    This method must be called before any operations with filesytem.

    \note Usually, this method calls internally and must not be called manually.
*/
void File::finit(const char *argv0) {
    if(!PHYSFS_init(argv0)) {
        aError() << "[ FileIO ] Can't initialize.";
    }
}
/*!
    Add an archive or directory to the search \a path.
    If \a writable provided as true the directory will be marked as writable.
    The Method can be called multiple time to add more directories to work with.

    \note Usually, this method calls internally and must not be called manually.
*/
void File::fsearchPathAdd(const char *path, bool writable) {
    if(PHYSFS_addToSearchPath(path, writable ? 0 : 1) == 0) {
        aError() << "[ FileIO ] Filed to add search path." << path << PHYSFS_getLastError();
    }
    if(writable && PHYSFS_setWriteDir(path) == 0) {
        aError() << "[ FileIO ] Can't set directory for writing.";
    }
}
/*!
    Get a file listing of a search \a path directory.

    \code
    StringList rc = file->_flist("savegames");

    for(auto it : rc) {
        printf("Found - [%s].\n", it.c_str());
    }
    \endcode
*/
StringList File::flist(const char *path) {
    char **rc = PHYSFS_enumerateFiles(path);
    char **i;

    StringList result;
    for(i = rc; *i != nullptr; i++) {
        result.push_back(*i);
    }

    PHYSFS_freeList(rc);

    return result;
}
/*!
    Create directory. Returns true if the operation succeeded; otherwise returns false.
    \note Directory can be created only if \a path marked as writable.
*/
bool File::mkdir(const char *path) {
    return (PHYSFS_mkdir(path) == 0);
}
/*!
    Delete file. Returns true if the operation succeeded; otherwise returns false.
    \note The file can be deleted only if \a path marked as writable.
*/
bool File::fdelete(const char *path) {
    bool result = (PHYSFS_delete(path) != 0);
    if(!result) {
        aError() << "[ FileIO ] Can't delete file" << path;
    }
    return result;
}
/*!
    Checks if a file by \a path exists. Returns true if operation succeeded; otherwise returns false.
*/
bool File::exists(const char *path) {
    return PHYSFS_exists(path);
}
/*!
    Determine if a file by \a path in the search path is really a directory.

    Returns true if operation succeeded; otherwise returns false.
*/
bool File::isdir(const char *path) {
    return PHYSFS_isDirectory(path);
}
/*!
    Closes file \a stream. Returns 0 if succeeded; otherwise returns non-zero value.
*/
int File::fclose(_FILE *stream) {
    return PHYSFS_close(static_cast<PHYSFS_file *>(stream));
}
/*!
    Seek to a new position within a file \a stream.
    Returns 0 if succeeded; otherwise returns non-zero value.
    The next read or write will occur at that \a origin position.
    Seeking past the beginning or end of the file is not allowed, and causes an error.

    \sa ftell()
*/
_size_t File::fseek(_FILE *stream, uint64_t origin) {
    A_UNUSED(origin);
    return static_cast<_size_t>(PHYSFS_seek(static_cast<PHYSFS_file *>(stream), origin));
}
/*!
    Opens the file whose name is specified in the \a path and associates it with a stream that can be identified in future operations.
    The operations that are allowed on the stream and how these are performed are defined by the \a mode parameter.
    Allowed values of \a mode parameter:
    \list
        \li "r" - Open a file for reading.
        \li "w" - Open a file for writing. The \a path must marked as writable.
        \li "a" - Open a file for appending. The \a path must marked as writable.
    \endlist

    Returns _FILE pointer to file stream if succeeded; otherwise returns nullptr value.
*/
_FILE *File::fopen(const char *path, const char *mode) {
    _FILE *result = nullptr;
    switch (mode[0]) {
        case 'r': result = static_cast<void *>(PHYSFS_openRead(path)); break;
        case 'w': result = static_cast<void *>(PHYSFS_openWrite(path)); break;
        case 'a': result = static_cast<void *>(PHYSFS_openAppend(path)); break;
        default: break;
    }
    if(result == nullptr) {
        aWarning() << "[ FileIO ] Can't open file" << path;
    }
    return result;
}
/*!
    Reads an array of \a count elements, each one with a size of \a size bytes, from the \a stream and stores them in the block of memory specified by \a ptr.
    The file must be opened for reading.

    Returns number of objects read.
*/
_size_t File::fread(void *ptr, _size_t size, _size_t count, _FILE *stream) {
    return static_cast<_size_t>(PHYSFS_read(static_cast<PHYSFS_file *>(stream), ptr, size, count));
}
/*!
    Writes an array of \a count elements, each one with a size of \a size bytes, from the block of memory pointed by \a ptr to the current position in the \a stream.
    The file must be opened for writing.

    Returns number of objects written.
*/
_size_t File::fwrite(const void *ptr, _size_t size, _size_t count, _FILE *stream) {
    return static_cast<_size_t>(PHYSFS_write(static_cast<PHYSFS_file *>(stream), ptr, size, count));
}
/*!
    Get total length of a file \a stream in bytes.
*/
_size_t File::fsize(_FILE *stream) {
    return static_cast<_size_t>(PHYSFS_fileLength(static_cast<PHYSFS_file *>(stream)));
}
/*!
    Determine current position within a file \a stream.

    Returns offset in bytes from start of file.
*/
_size_t File::ftell(_FILE *stream) {
    return static_cast<_size_t>(PHYSFS_tell(static_cast<PHYSFS_file *>(stream)));
}
