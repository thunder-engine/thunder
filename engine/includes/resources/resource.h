#ifndef RESOURCE_H
#define RESOURCE_H

#include "engine.h"

class ResourcePrivate;
class ResourceSystem;

class Component;

class ENGINE_EXPORT Resource : public Object {
    A_REGISTER(Resource, Object, General)

    A_NOPROPERTIES()
    A_NOMETHODS()

public:
    enum ResourceState {
        Invalid,
        Loading,
        ToBeUpdated,
        Ready,
        Suspend,
        Unloading,
        ToBeDeleted
    };

    class ENGINE_EXPORT IObserver {
    public:
        virtual ~IObserver() {}
        virtual void resourceUpdated(const Resource *resource, ResourceState state) = 0;
    };

public:
    Resource();
    ~Resource() override;

    ResourceState state() const;

    void incRef();
    void decRef();

    void subscribe(IObserver *observer);
    void unsubscribe(IObserver *observer);

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
