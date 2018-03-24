#ifndef COMPONENT_H
#define COMPONENT_H

#include <engine.h>

class Actor;

class NEXT_LIBRARY_EXPORT Component : public Object {
    A_REGISTER(Component, Object, Components);

public:
    Component                   ();

    virtual void                update                  ();

    virtual bool                isEnable                () const;

    Actor                      &actor                   () const;
};

#endif // COMPONENT_H
