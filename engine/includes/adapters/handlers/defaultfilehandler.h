#ifndef DEFAULTFILEHANDLER_H
#define DEFAULTFILEHANDLER_H

#include <file.h>
#include <log.h>

#include <stdio.h>
#include <filesystem>

class DefaultFileHandler : public FileHandler {
public:
    void clearSearchPaths() {
        m_searchPath.clear();
    }

    void searchPathAdd(const TString &path) {
        m_searchPath.push_back(path);
    }

protected:
    StringList list(const char *path) override {
        StringList result;
        for(auto const &it : std::filesystem::directory_iterator{path}) {
            result.push_back(TString(it.path().string()));
        }
        return result;
    }

    bool mkDir(const char *path) override {
        return std::filesystem::create_directory(path);
    }

    bool remove(const char *path) override {
        return std::filesystem::remove(path);
    }

    bool exists(const char *path) override {
        return std::filesystem::exists(path);
    }

    bool isDir(const char *path) override {
        return std::filesystem::is_directory(path);
    }

    bool isFile(const char *path) override {
        return std::filesystem::is_regular_file(path);
    }

    int close(int *handle) override {
        return ::fclose(reinterpret_cast<FILE *>(handle));
    }

    size_t seek(int *handle, uint64_t origin) override {
        return ::fseek(reinterpret_cast<FILE *>(handle), origin, SEEK_SET);
    }

    int *open(const char *path, int mode) override {
        TString s;
        if(mode & File::ReadOnly) {
            s += 'r';
        }

        if(mode & File::WriteOnly) {
            s += 'w';
        }

        if(mode & File::Append) {
            s += 'a';
        }

        s += 'b';

        FILE *fp = ::fopen(path, s.data());
        if(fp == nullptr) {
            for(auto &it : m_searchPath) {
                fp = ::fopen((it + "/" + path).data(), s.data());
                if(fp) {
                    return reinterpret_cast<int *>(fp);
                }
            }

            aWarning() << "[DefaultFileHandler] Can't open file" << path;
        }

        return reinterpret_cast<int *>(fp);
    }

    size_t read(void *ptr, size_t size, size_t count, int *handle) override {
        return ::fread(ptr, size, count, reinterpret_cast<FILE *>(handle));
    }

    size_t write(const void *ptr, size_t size, size_t count, int *handle) override {
       return ::fwrite(ptr, size, count, reinterpret_cast<FILE *>(handle));
    }

    size_t size(int *handle) override {
        ::fseek(reinterpret_cast<FILE *>(handle), 0, SEEK_END);
        size_t size = ::ftell(reinterpret_cast<FILE *>(handle));
        ::fseek(reinterpret_cast<FILE *>(handle), 0, SEEK_SET);

        return size;
    }

    size_t tell(int *handle) override {
        return ::ftell(reinterpret_cast<FILE *>(handle));
    }

protected:
    StringList m_searchPath;

};

#endif // DEFAULTFILEHANDLER_H
