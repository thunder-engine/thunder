#ifndef LIGHTSOURCEGL_H
#define LIGHTSOURCEGL_H

#include <amath.h>
#include <components/lightsource.h>

#include <engine.h>

#include "resources/atexturegl.h"

class AShaderGL;
class APipeline;

class ALightSourceGL : public LightSource {
    A_OVERRIDE(ALightSourceGL, LightSource, Components)

    A_NOMETHODS()
    A_NOPROPERTIES()

public:
    ALightSourceGL              ();

    void                        draw                (APipeline &pipeline, uint8_t layer);
    void                        shadowsUpdate       (APipeline &pipeline);

    void                        setShaderParams     (uint32_t program);

protected:
    void                        lightDirect         (APipeline &pipeline, uint8_t layer);
    void                        lightPoint          (APipeline &pipeline);
    void                        lightSpot           (APipeline &pipeline);

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
    Vector3                   m_Dir;
    /// Light source matrix array.
    Matrix4                  *m_pMatrix;
    /// Distance of cutting for each level of LOD.
    Vector3                   m_Distance;
    /// Field of view for all LOD levels
    Vector4                   m_LOD;
};

#endif // LIGHTSOURCEGL_H
