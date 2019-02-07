#ifndef ANDROIDFILE_H
#define ANDROIDFILE_H

#include "file.h"

#include <glfm.h>

class AndroidFile : public IFile {
public:
    StringList  _flist          (const char *path) {
        AAssetDir *dir = AAssetManager_openDir(glfmAndroidGetActivity()->assetManager, path);
        StringList result;
        const char *name = nullptr;
        while((name = AAssetDir_getNextFileName(dir)) != nullptr) {
            result.push_back(name);
        }
        AAssetDir_close(dir);
        return result;
    }

    bool        _mkdir          (const char *path) {
        A_UNUSED(path);
        return false;
    }

    bool        _delete         (const char *path) {
        A_UNUSED(path);
        return false;
    }

    bool        _exists         (const char *path) {
        A_UNUSED(path);
        return true;
    }

    bool        _isdir          (const char *path) {
        AAssetDir *dir = AAssetManager_openDir(glfmAndroidGetActivity()->assetManager, path);
        if(dir) {
             AAssetDir_close(dir);
             return true;
        }
        return false;
    }

    int         _fclose         (_FILE *stream) {
        AAsset_close((AAsset *)stream);
        return 0;
    }

    _size_t     _fseek          (_FILE *stream, long int offset, int origin) {
        return AAsset_seek((AAsset *)stream, offset, origin);
    }

    _FILE      *_fopen          (const char *path, const char *mode) {
        return AAssetManager_open(glfmAndroidGetActivity()->assetManager, path, AASSET_MODE_UNKNOWN);
    }

    _size_t     _fread          (void *ptr, _size_t size, _size_t count, _FILE *stream) {
        return AAsset_read((AAsset *)stream, ptr, size);
    }

    _size_t     _fwrite         (const void *ptr, _size_t size, _size_t count, _FILE *stream) {
        A_UNUSED(ptr);
        A_UNUSED(size);
        A_UNUSED(count);
        A_UNUSED(stream);
       return -1;
    }

    _size_t     _fsize          (_FILE *stream) {
        return AAsset_getLength((AAsset *)stream);
    }

    _size_t     _ftell          (_FILE *stream) {
        return _fsize(stream) - AAsset_getRemainingLength((AAsset *)stream);
    }

};

#endif // ANDROIDFILE_H
