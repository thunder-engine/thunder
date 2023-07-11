#ifndef RESOURCE_H
#define RESOURCE_H

#include "engine.h"

class ResourcePrivate;
class ResourceSystem;

class Component;

enum ResourceState {
    Invalid,
    Loading,
    ToBeUpdated,
    Ready,
    Suspend,
    Unloading,
    ToBeDeleted
};

class ENGINE_EXPORT Resource : public Object {
    A_REGISTER(Resource, Object, General)

    A_NOPROPERTIES()
    A_NOMETHODS()

public:
    typedef void (*ResourceUpdatedCallback)(int state, void *ptr);

public:
    Resource();
    ~Resource() override;

    ResourceState state() const;

    void incRef();
    void decRef();

    void subscribe(ResourceUpdatedCallback callback, void *ptr);
    void unsubscribe(void *ptr);

protected:
    virtual void switchState(ResourceState state);
    virtual bool isUnloadable();
    void setState(ResourceState state);

    void notifyCurrentState();

private:
    friend class ResourceSystem;

    ResourcePrivate *p_ptr;

};

#endif // RESOURCE_H
