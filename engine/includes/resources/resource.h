#ifndef RESOURCE_H
#define RESOURCE_H

#include "engine.h"

class ResourcePrivate;
class ResourceSystem;

class Component;

class NEXT_LIBRARY_EXPORT Resource : public Object {
    A_REGISTER(Resource, Object, General)
public:
    enum ResourceState {
        Invalid,
        Loading,
        ToBeUpdated,
        Ready,
        Suspend,
        ToBeDeleted
    };

    class NEXT_LIBRARY_EXPORT IObserver {
    public:
        virtual ~IObserver() {}
        virtual void resourceUpdated(const Resource *resource, ResourceState state) = 0;
    };

public:
    Resource ();
    ~Resource () override;

    ResourceState state() const;

    void incRef();
    void decRef();

    void subscribe (IObserver *observer);
    void unsubscribe (IObserver *observer);

protected:
    virtual void setState(ResourceState state);

private:
    friend class ResourceSystem;

    ResourcePrivate *p_ptr;

};

#endif // RESOURCE_H
