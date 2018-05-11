#ifndef ADIRECTLIGHTGL_H
#define ADIRECTLIGHTGL_H

#include <amath.h>
#include <components/directlight.h>

#include <engine.h>

#include "resources/atexturegl.h"

class ICommandBuffer;
class Material;
class MaterialInstance;
class Mesh;

class APipeline;

class ADirectLightGL : public DirectLight {
    A_OVERRIDE(ADirectLightGL, DirectLight, Components)

    A_NOMETHODS()
    A_NOPROPERTIES()

public:
    void                        shadowsUpdate       (APipeline &pipeline);

};

#endif // ADIRECTLIGHTGL_H
