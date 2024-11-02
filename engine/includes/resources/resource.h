#ifndef RESOURCE_H
#define RESOURCE_H

#include "engine.h"

#include <mutex>



class ENGINE_EXPORT Resource : public Object {
    A_REGISTER(Resource, Object, General)

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
    ~Resource() override;

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
