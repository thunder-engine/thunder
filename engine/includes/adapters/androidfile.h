#ifndef ANDROIDFILE_H
#define ANDROIDFILE_H

#include "file.h"

#include <glfm.h>

class AndroidFile : public File {
public:
    StringList flist(const char *path) override {
        AAssetDir *dir = AAssetManager_openDir(glfmAndroidGetActivity()->assetManager, path);
        StringList result;
        const char *name = nullptr;
        while((name = AAssetDir_getNextFileName(dir)) != nullptr) {
            result.push_back(name);
        }
        AAssetDir_close(dir);
        return result;
    }

    bool mkdir(const char *path) override {
        A_UNUSED(path);
        return false;
    }

    bool fdelete(const char *path) override {
        A_UNUSED(path);
        return false;
    }

    bool exists(const char *path) override {
        A_UNUSED(path);
        return true;
    }

    bool isdir(const char *path) override {
        AAssetDir *dir = AAssetManager_openDir(glfmAndroidGetActivity()->assetManager, path);
        if(dir) {
             AAssetDir_close(dir);
             return true;
        }
        return false;
    }

    int fclose(_FILE *stream) override {
        AAsset_close((AAsset *)stream);
        return 0;
    }

    _size_t fseek(_FILE *stream, uint64_t origin) override {
        return AAsset_seek((AAsset *)stream, origin, SEEK_SET);
    }

    _FILE *fopen(const char *path, const char *mode) override {
        return AAssetManager_open(glfmAndroidGetActivity()->assetManager, path, AASSET_MODE_UNKNOWN);
    }

    _size_t fread(void *ptr, _size_t size, _size_t count, _FILE *stream) override {
        return AAsset_read((AAsset *)stream, ptr, size);
    }

    _size_t fwrite(const void *ptr, _size_t size, _size_t count, _FILE *stream) override {
        A_UNUSED(ptr);
        A_UNUSED(size);
        A_UNUSED(count);
        A_UNUSED(stream);
       return -1;
    }

    _size_t fsize(_FILE *stream) override {
        return AAsset_getLength((AAsset *)stream);
    }

    _size_t ftell(_FILE *stream) override {
        return fsize(stream) - AAsset_getRemainingLength((AAsset *)stream);
    }

};

#endif // ANDROIDFILE_H
