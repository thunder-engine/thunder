#ifndef ANDROIDFILE_H
#define ANDROIDFILE_H

#include <file.h>
#include <systems/resourcesystem.h>

#include <glfm.h>

class AndroidFileHandler : public FileHandler {
public:
    StringList list(const char *path) override {
        AAssetDir *dir = AAssetManager_openDir(glfmAndroidGetActivity()->assetManager, path);
        StringList result;
        const char *name = nullptr;
        while((name = AAssetDir_getNextFileName(dir)) != nullptr) {
            result.push_back(name);
        }
        AAssetDir_close(dir);
        return result;
    }

    bool mkDir(const char *path) override {
        A_UNUSED(path);
        return false;
    }

    bool remove(const char *path) override {
        A_UNUSED(path);
        return false;
    }

    void rename(const char *origin, const char *target) override {
        A_UNUSED(origin);
        A_UNUSED(target);
    };

    void copy(const char *origin, const char *target) override {
        A_UNUSED(origin);
        A_UNUSED(target);
    };

    bool exists(const char *path) override {
        A_UNUSED(path);
        return true;
    }

    bool isDir(const char *path) override {
        AAssetDir *dir = AAssetManager_openDir(glfmAndroidGetActivity()->assetManager, path);
        if(dir) {
             AAssetDir_close(dir);
             return true;
        }
        return false;
    }

    bool isFile(const char *path) override {
        A_UNUSED(path);
        return true;
    }

    int close(int *handle) override {
        AAsset_close(reinterpret_cast<AAsset *>(handle));
        return 0;
    }

    size_t seek(int *handle, uint64_t origin) override {
        return AAsset_seek(reinterpret_cast<AAsset *>(handle), origin, SEEK_SET);
    }

    int *open(const char *path, int) override {
        return reinterpret_cast<int *>(AAssetManager_open(glfmAndroidGetActivity()->assetManager, path, AASSET_MODE_UNKNOWN));
    }

    size_t read(void *ptr, size_t size, size_t count, int *handle) override {
        return AAsset_read(reinterpret_cast<AAsset *>(handle), ptr, size);
    }

    size_t write(const void *ptr, size_t size, size_t count, int *handle) override {
        A_UNUSED(ptr);
        A_UNUSED(size);
        A_UNUSED(count);
        A_UNUSED(handle);
       return -1;
    }

    size_t size(int *handle) override {
        return AAsset_getLength(reinterpret_cast<AAsset *>(handle));
    }

    size_t tell(int *handle) override {
        return size(handle) - AAsset_getRemainingLength(reinterpret_cast<AAsset *>(handle));
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

#endif // ANDROIDFILE_H
