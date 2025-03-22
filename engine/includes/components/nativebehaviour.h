#ifndef NATIVEBEHAVIOUR_H
#define NATIVEBEHAVIOUR_H

#include "component.h"

class ENGINE_EXPORT NativeBehaviour : public Component {
    A_OBJECT(NativeBehaviour, Component, General)

    A_NOPROPERTIES()
    A_NOMETHODS()

public:
    NativeBehaviour();

    ~NativeBehaviour();

    virtual void start();

    virtual void update();

protected:
    void setSystem(ObjectSystem *system) override;

};

#endif // NATIVEBEHAVIOUR_H
