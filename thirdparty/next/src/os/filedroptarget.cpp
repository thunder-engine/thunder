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

#include "os/filedroptarget.h"

void FileDropTarget::setDropHandler(const DropHandler &handler) {
    m_handler = handler;
}

bool FileDropTarget::acceptsDrops() const {
    return static_cast<bool>(m_handler);
}

bool FileDropTarget::deliverFiles(const StringList &files) const {
    if(files.empty() || !m_handler) {
        return false;
    }

    m_handler(files);
    return true;
}
