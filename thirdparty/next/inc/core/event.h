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

#ifndef EVENT_H
#define EVENT_H

#include <stdint.h>

#include <global.h>

class NEXT_LIBRARY_EXPORT Event {
public:
    enum Type {
        Invalid = 0,
        MethodCall,
        TimerEvent,
        Destroy,
        LanguageChange,
        FileSystemWatcher,
        UserType = 100
    };

public:
    Event(uint32_t type);

    virtual ~Event();

    uint32_t type() const;

protected:
    uint32_t m_type;

};

#endif // EVENT_H
