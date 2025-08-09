#ifndef FILE_H
#define FILE_H

#include <stdint.h>

#include <astring.h>

class FileHandler;

class NEXT_LIBRARY_EXPORT File {
public:
    enum OpenMode {
        ReadOnly    = (1<<0),
        WriteOnly   = (1<<1),
        Append      = (1<<2),
        Text        = (1<<4)
    };

public:
    explicit File(const TString &filename);
    ~File();

    TString fileName() const;
    void setFileName(const TString &filename);

    bool open(int openMode);

    bool exists() const;

    void close();

    size_t read(void *ptr, size_t size, size_t count) const;

    ByteArray readAll() const;

    size_t write(const TString &data);

    size_t write(const ByteArray &data);

    bool seek(size_t offset);

    size_t size() const;

    size_t pos() const;

    static void setHandler(FileHandler *handler);

    static FileHandler *handler();

    static bool exists(const TString &filename);

    static StringList list(const TString &path);

    static bool isFile(const TString &path);

protected:
    friend class FileHandler;

    TString m_fileName;

    int *m_handle;

};

class NEXT_LIBRARY_EXPORT FileHandler {
public:
    virtual StringList list(const char *path) = 0;

    virtual bool mkDir(const char *path) = 0;

    virtual bool remove(const char *path) = 0;

    virtual bool exists(const char *path) = 0;

    virtual bool isDir(const char *path) = 0;

    virtual bool isFile(const char *path) = 0;

    virtual int close(int *handle) = 0;

    virtual int *open(const char *path, int mode) = 0;

    virtual size_t seek(int *handle, uint64_t origin) = 0;

    virtual size_t read(void *ptr, size_t size, size_t count, int *handle) = 0;

    virtual size_t write(const void *ptr, size_t size, size_t count, int *handle) = 0;

    virtual size_t size(int *handle) = 0;

    virtual size_t tell(int *handle) = 0;

};

#endif // FILE_H
