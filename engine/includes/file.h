#ifndef FILE_H
#define FILE_H

#include <stdint.h>
#include <string>
#include <list>

#include <engine.h>

using namespace std;

typedef void            _FILE;
typedef	uint64_t        _size_t;
typedef list<string>    StringList;

class ENGINE_EXPORT File {
public:
    void finit(const char *argv0);
    void fsearchPathAdd(const char *path, bool isFirst = false);

    virtual StringList flist(const char *path);

    virtual bool mkdir(const char *path);

    virtual bool fdelete(const char *path);

    virtual bool exists(const char *path);

    virtual bool isdir(const char *path);

    virtual int fclose(_FILE *stream);

    virtual _size_t fseek(_FILE *stream, uint64_t origin);

    virtual _FILE *fopen(const char *path, const char *mode);

    virtual _size_t fread(void *ptr, _size_t size, _size_t count, _FILE *stream);

    virtual _size_t fwrite(const void *ptr, _size_t size, _size_t count, _FILE *stream);

    virtual _size_t fsize(_FILE *stream);

    virtual _size_t ftell(_FILE *stream);
};

#endif // FILE_H
