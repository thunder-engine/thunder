#ifndef DIRECTLIGHT_H
#define DIRECTLIGHT_H

#include "baselight.h"

class ENGINE_EXPORT DirectLight : public BaseLight {
    A_OBJECT(DirectLight, BaseLight, Components/Lights)

    A_NOPROPERTIES()
    A_NOMETHODS()

public:
    DirectLight();

private:
    int lightType() const override;

    void drawGizmos() override;

};

#endif // DIRECTLIGHT_H
