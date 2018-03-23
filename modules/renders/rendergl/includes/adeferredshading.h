#ifndef ADEFERREDSHADING_H
#define ADEFERREDSHADING_H

#include "apipeline.h"

#include "resources/atexturegl.h"

class Engine;
class Shader;

class ADeferredShading : public APipeline {
public:
    enum g_components_types {
        G_NORMALS               = 0,
        G_DIFFUSE,
        G_PARAMS,
        G_EMISSIVE,
        G_TARGETS
    };

    ADeferredShading            (Engine *engine);
    ~ADeferredShading           ();

    /*!
        Preparation of texture buffers.
    */
    void                        draw                (Scene &scene, uint32_t resource);

    /*!
        Resizing current texture buffers.
        @param[in]  width       New screen width.
        @param[in]  height      New screen height.
    */
    void                        resize              (uint32_t width, uint32_t height);

private:
    bool                        m_HDR;

    uint32_t                    fb_g_id;
    uint32_t                    fb_s_id;

    ATextureGL                 *m_pGBuffer;
    ATextureGL                  m_DepthMap;

    Mesh                       *m_pPlane;
    MaterialInstance           *m_pSprite;
};

#endif // ADEFERREDSHADING_H
