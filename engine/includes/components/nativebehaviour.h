#ifndef NATIVEBEHAVIOUR_H
#define NATIVEBEHAVIOUR_H

#include "component.h"

class ENGINE_EXPORT NativeBehaviour : public Component {
    A_REGISTER(NativeBehaviour, Component, General)

    A_NOPROPERTIES()
    A_NOMETHODS()

public:
    NativeBehaviour();

    virtual void start();

    virtual void update();

};

#endif // NATIVEBEHAVIOUR_H
