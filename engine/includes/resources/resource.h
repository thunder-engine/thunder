#ifndef RESOURCE_H
#define RESOURCE_H

#include "engine.h"

class ResourcePrivate;
class ResourceSystem;

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

public:
    Resource ();
    ~Resource () override;

    ResourceState state() const;

    void incRef();
    void decRef();

protected:
    void setState(ResourceState state);

private:
    friend class ResourceSystem;

    ResourcePrivate *p_ptr;

};

#endif // RESOURCE_H
