#ifndef APIPELINE
#define APIPELINE

#include <cstdint>
#include <stack>

#include <amath.h>
#include <controller.h>

#include "resources/atexturegl.h"
#include "resources/amaterialgl.h"
#include "resources/ameshgl.h"

class Engine;
class BaseController;

class ATextureGL;
class ACameraGL;
class ASpriteGL;

class ABlurGL;

class AAmbientOcclusionGL;

class APostProcessor;

class ICommandBuffer;

class IDrawObject {
public:
    virtual void                draw                (ICommandBuffer &buffer, int8_t layer) = 0;
};

class APipeline {
public:
    APipeline                   (Engine *engine);

    virtual ~APipeline          ();

    virtual void                draw                (Scene &scene, uint32_t);

    virtual ATextureGL         *postProcess         (ATextureGL &source);

    virtual void                resize              (uint32_t width, uint32_t height);

    void                        drawComponents      (AObject &object, uint8_t layer);

    Vector2                     screen              () const { return m_Screen; }
    Vector3                     world               () const { return m_World; }

    void                        cameraReset         ();
    Camera                     *activeCamera        ();

    void                        overrideController  (IController *controller) { m_pController = controller; }

protected:
    void                        updateLights        (AObject &object, uint8_t layer);
    void                        updateShadows       (AObject &object);

    void                        analizeScene        (AObject &object);

protected:
    ICommandBuffer             *m_Buffer;

    IController                *m_pController;

    Engine                     *m_pEngine;

    Scene                      *m_pScene;

    ATextureGL                  m_Select;
    ATextureGL                  m_Depth;
    /// Frame buffers
    uint32_t                    m_SelectBuffer;

    Vector2                     m_Screen;

    Vector3                     m_World;

    list<APostProcessor *>      m_PostEffects;

};

#endif // APIPELINE
