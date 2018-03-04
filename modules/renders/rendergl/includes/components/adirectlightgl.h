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

class ADirectLightGL : public DirectLight {
    A_OVERRIDE(ADirectLightGL, DirectLight, Components)

    A_NOMETHODS()
    A_NOPROPERTIES()

public:
    ADirectLightGL              ();

    void                        draw                (ICommandBuffer &buffer, uint8_t layer);
    void                        shadowsUpdate       (ICommandBuffer &buffer);

    void                        setShaderParams     (uint32_t program);

protected:
    /// Number of levels of detail shadows. 0 - if shadows are disabled.
    uint8_t                     m_LODCount;

    ATextureGL                  m_ShadowMap;
    ATextureGL                  m_ShadowTemp;


    uint8_t                     m_Steps;
    float                      *m_pPoints;

    /// Resolution depth map.
    unsigned short              m_Resolution;
    /// Light source are using the Cascaded Shadow Maps.
    bool                        m_CSM;
    Vector3                     m_Dir;
    /// Light source matrix array.
    Matrix4                    *m_pMatrix;
    /// Distance of cutting for each level of LOD.
    Vector3                     m_Distance;
    /// Field of view for all LOD levels
    Vector4                     m_LOD;

    Mesh                       *m_pPlane;

    Material                   *m_pMaterial;
    MaterialInstance           *m_pMaterialInstance;
};

#endif // ADIRECTLIGHTGL_H
