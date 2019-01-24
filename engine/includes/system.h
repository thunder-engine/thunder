#ifndef ABSTRACTSYSTEM_H
#define ABSTRACTSYSTEM_H

#include <string>
#include <stdint.h>

#include <engine.h>

class Scene;

using namespace std;

class ISystem : public ObjectSystem {
public:
    ISystem                     () {}
    virtual ~ISystem            () {}

    virtual bool                init                        () = 0;

    virtual const char         *name                        () const = 0;

    virtual void                update                      (Scene *scene) = 0;
};

#endif // ABSTRACTSYSTEM_H
