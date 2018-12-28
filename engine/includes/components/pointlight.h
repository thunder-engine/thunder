#ifndef POINTLIGHT_H
#define POINTLIGHT_H

#include "baselight.h"

class Mesh;
class MaterialInstance;

class NEXT_LIBRARY_EXPORT PointLight : public BaseLight {
    A_REGISTER(PointLight, BaseLight, Components)

    A_NOPROPERTIES()

public:
    PointLight                  ();

    ~PointLight                 ();

    void                        draw                    (ICommandBuffer &buffer, int8_t layer);

protected:


};

#endif // POINTLIGHT_H
