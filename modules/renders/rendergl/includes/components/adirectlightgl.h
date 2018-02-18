#ifndef ADIRECTLIGHTGL_H
#define ADIRECTLIGHTGL_H

#include <amath.h>
#include <components/directlight.h>

#include <engine.h>

#include "resources/atexturegl.h"

class AMaterialGL;
class APipeline;
class MaterialInstance;

class ADirectLightGL : public DirectLight {
    A_OVERRIDE(ADirectLightGL, DirectLight, Components)

    A_NOMETHODS()
    A_NOPROPERTIES()

public:
    ADirectLightGL              ();

    void                        draw                (APipeline &pipeline, uint8_t layer);
    void                        shadowsUpdate       (APipeline &pipeline);

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

    AMaterialGL                *m_pMaterial;
    MaterialInstance           *m_pMaterialInstance;
};

#endif // ADIRECTLIGHTGL_H
