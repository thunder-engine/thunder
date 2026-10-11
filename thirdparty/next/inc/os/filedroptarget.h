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

#ifndef FILEDROPTARGET_H
#define FILEDROPTARGET_H

#include <astring.h>

#include <functional>

namespace Next {
namespace OS {

class NEXT_LIBRARY_EXPORT FileDropTarget {
public:
    using DropHandler = std::function<void(const StringList &)>;

    FileDropTarget() = default;

    // Set an empty handler to stop accepting OS file drops.
    void setDropHandler(const DropHandler &handler);
    bool acceptsDrops() const;
    bool deliverFiles(const StringList &files) const;

private:
    DropHandler m_handler;
};

} // namespace OS
} // namespace Next

#endif // FILEDROPTARGET_H
