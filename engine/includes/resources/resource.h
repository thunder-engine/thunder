/*
    This file is part of Thunder Engine.

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

#ifndef RESOURCE_H
#define RESOURCE_H

#include "engine.h"

#include <mutex>

class ENGINE_EXPORT Resource : public Object {
    A_OBJECT(Resource, Object, General)

    A_NOPROPERTIES()
    A_NOMETHODS()

public:
    enum State {
        Invalid,
        Loading,
        ToBeUpdated,
        Ready,
        Suspend,
        Unloading,
        ToBeDeleted
    };

public:
    typedef void (*ResourceUpdatedCallback)(int state, void *object);

public:
    Resource();
    Resource(const Resource &origin);

    State state() const;

    void incRef();
    void decRef();

    void subscribe(ResourceUpdatedCallback callback, void *object);
    void unsubscribe(void *object);

protected:
    virtual void switchState(State state);
    virtual bool isUnloadable();
    void setState(State state);

    void notifyCurrentState();

private:
    friend class ResourceSystem;

    typedef std::list<std::pair<Resource::ResourceUpdatedCallback, void *>> Callbacks;

    Callbacks m_observers;

    State m_state;
    State m_last;

    uint32_t m_referenceCount;

    std::mutex m_mutex;

};

#endif // RESOURCE_H
