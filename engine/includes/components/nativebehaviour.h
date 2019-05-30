#ifndef NATIVEBEHAVIOUR_H
#define NATIVEBEHAVIOUR_H

#include "component.h"

class NEXT_LIBRARY_EXPORT NativeBehaviour : public Component {
    A_REGISTER(NativeBehaviour, Component, General)

    A_NOPROPERTIES()
    A_NOMETHODS()

public:
    virtual void                start                   ();

    virtual void                update                  ();

};

#endif // NATIVEBEHAVIOUR_H
