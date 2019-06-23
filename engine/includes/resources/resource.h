#ifndef RESOURCE_H
#define RESOURCE_H

#include "engine.h"

class ResourcePrivate;

class NEXT_LIBRARY_EXPORT Resource : public Object {
    A_REGISTER(Resource, Object, General)

    Resource ();
    virtual ~Resource ();

    bool isValid();
    void setValid(bool valid);

private:
    ResourcePrivate *p_ptr;

};

#endif // RESOURCE_H
