#include "file.h"

#include "log.h"

static FileHandler *s_handler = nullptr;
/*

*/

File::File(const TString &filename) :
        m_fileName(filename),
        m_handle(nullptr) {

}

File::~File() {
    close();
}

TString File::fileName() const {
    return m_fileName;
}

void File::setFileName(const TString &filename) {
    m_fileName = filename;
}

bool File::open(int openMode) {
    return (m_handle = s_handler->open(m_fileName.data(), openMode)) != nullptr;
}

bool File::exists() const {
    return s_handler->exists(m_fileName.data());
}

void File::close() {
    if(m_handle) {
        s_handler->close(m_handle);
        m_handle = nullptr;
    }
}

size_t File::read(void *ptr, size_t size, size_t count) const {
    return s_handler->read(ptr, size, count, m_handle);
}

ByteArray File::readAll() const {
    ByteArray result;

    result.resize(size());
    s_handler->read(result.data(), result.size(), 1, m_handle);

    return result;
}

size_t File::write(const TString &data) {
    return s_handler->write(data.data(), data.size(), 1, m_handle);
}

size_t File::write(const ByteArray &data) {
    return s_handler->write(data.data(), data.size(), 1, m_handle);
}

size_t File::write(const char *ptr, size_t size) {
    return s_handler->write(ptr, size, 1, m_handle);
}

bool File::seek(size_t offset) {
    return s_handler->seek(m_handle, offset) == 0;
}

size_t File::size() const {
    return s_handler->size(m_handle);
}

size_t File::pos() const {
    return s_handler->tell(m_handle);
}

void File::setHandler(FileHandler *handler) {
    s_handler = handler;
}

FileHandler *File::handler() {
    return s_handler;
}

bool File::exists(const TString &file) {
    return s_handler->exists(file.data());
}

bool File::remove(const TString &file) {
    return s_handler->remove(file.data());
}

bool File::rename(const TString &origin, const TString &target) {
    return s_handler->rename(origin.data(), target.data());
}

bool File::copy(const TString &origin, const TString &target) {
    return s_handler->copy(origin.data(), target.data());
}

StringList File::list(const TString &path) {
    return s_handler->list(path.data());
}

bool File::isFile(const TString &path) {
    return s_handler->isFile(path.data());
}

bool File::isDir(const TString &path) {
    return s_handler->isDir(path.data());
}

/*!
    Returns the MD5 hash for the \a file.
    \note This function calculates hash sum in editor mode and returns cached sum for the resource in game mode.
*/
TString File::md5(const TString &file) {
    return s_handler->md5(file.data());
}
