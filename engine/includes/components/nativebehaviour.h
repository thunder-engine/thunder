#ifndef NATIVEBEHAVIOUR_H
#define NATIVEBEHAVIOUR_H

#include "components/component.h"

#include "system.h"

class NEXT_LIBRARY_EXPORT NativeBehaviour : public Component {
    A_REGISTER(NativeBehaviour, Component, Components)

    A_NOPROPERTIES()
    A_NOMETHODS()

public:
    virtual void                start                   ();

    virtual void                update                  ();

};

#endif // NATIVEBEHAVIOUR_H
