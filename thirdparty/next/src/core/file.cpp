/*
    This file is part of Thunder Next.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "file.h"

static FileHandler *s_handler = nullptr;

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
    return (s_handler && (m_handle = s_handler->open(m_fileName.data(), openMode)) != nullptr);
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

void File::mount(const TString &path, bool writable) {
    s_handler->mount(path.data(), writable);
}

void File::unmount(const TString &path) {
    s_handler->unmount(path.data());
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

bool File::mkDir(const TString &path) {
    return s_handler->mkDir(path.data());
}

bool File::mkPath(const TString &path) {
    return s_handler->mkPath(path.data());
}

StringList File::list(const TString &path, bool recursive) {
    return s_handler->list(path.data(), recursive);
}

bool File::isFile(const TString &path) {
    return s_handler->isFile(path.data());
}

bool File::isDir(const TString &path) {
    return s_handler->isDir(path.data());
}
