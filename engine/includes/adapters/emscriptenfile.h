#ifndef EMSCRIPTENFILE_H
#define EMSCRIPTENFILE_H

#include "file.h"
#include "log.h"

#include <stdio.h>
#include <filesystem>

class EmscriptenFile : public File {
public:
    StringList flist(const char *path) override {
        StringList result;
        for(auto const &dir_entry : std::filesystem::directory_iterator{path}) {
            result.push_back(TString(dir_entry.path()));
        }
        return result;
    }

    bool mkdir(const char *path) override {
        return std::filesystem::create_directory(path);
    }

    bool fdelete(const char *path) override {
        return std::filesystem::remove(path);
    }

    bool exists(const char *path) override {
        return std::filesystem::exists(path);
    }

    bool isdir(const char *path) override {
        return std::filesystem::is_directory(path);
    }

    int fclose(_FILE *stream) override {
        return ::fclose((FILE *)stream);
    }

    _size_t fseek(_FILE *stream, uint64_t origin) override {
        return ::fseek((FILE *)stream, origin, SEEK_SET);
    }

    _FILE *fopen(const char *path, const char *mode) override {
        FILE *fp = ::fopen(path, mode);
        if(fp == nullptr) {
            aWarning() << "[EmscriptenFile] Can't open file" << path;
        }

        return fp;
    }

    _size_t fread(void *ptr, _size_t size, _size_t count, _FILE *stream) override {
        return ::fread(ptr, size, count, (FILE *)stream);
    }

    _size_t fwrite(const void *ptr, _size_t size, _size_t count, _FILE *stream) override {
       return ::fwrite(ptr, size, count, (FILE *)stream);
    }

    _size_t fsize(_FILE *stream) override {
        if(stream) {
            ::fseek((FILE *)stream, 0, SEEK_END);
            _size_t size = ::ftell((FILE *)stream);
            ::fseek((FILE *)stream, 0, SEEK_SET);

            return size;
        }
        return 0;
    }

    _size_t ftell(_FILE *stream) override {
        return ::ftell((FILE *)stream);
    }

};

#endif // EMSCRIPTENFILE_H
