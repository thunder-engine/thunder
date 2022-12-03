#ifndef DIRECTLIGHT_H
#define DIRECTLIGHT_H

#include "baselight.h"

class ENGINE_EXPORT DirectLight : public BaseLight {
    A_REGISTER(DirectLight, BaseLight, Components/Lights)

    A_NOPROPERTIES()
    A_NOMETHODS()

public:
    DirectLight();

private:
    int lightType() const override;

#ifdef SHARED_DEFINE
    bool drawHandles(ObjectList &selected) override;
#endif

};

#endif // DIRECTLIGHT_H
