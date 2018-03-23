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
    ADirectLightGL              ();

    ~ADirectLightGL             ();

    void                        draw                (APipeline &pipeline, uint8_t layer);
    void                        shadowsUpdate       (APipeline &pipeline);

protected:
    /// Number of levels of detail shadows. 0 - if shadows are disabled.
    uint8_t                     m_LODCount;

    ATextureGL                 *m_pDepthMap;

    uint32_t                    m_DepthBuffer;

    /// Resolution depth map.
    uint16_t                    m_Resolution;
    /// Light source are using the Cascaded Shadow Maps.
    bool                        m_CSM;
    /// Light source matrix array.
    Matrix4                    *m_pMatrix;
    /// Distance of cutting for each level of LOD.
    Vector4                     m_Distance;
    /// Field of view for all LOD levels
    Vector4                     m_LOD;

    Mesh                       *m_pPlane;

    Material                   *m_pMaterial;
    MaterialInstance           *m_pMaterialInstance;
};

#endif // ADIRECTLIGHTGL_H
