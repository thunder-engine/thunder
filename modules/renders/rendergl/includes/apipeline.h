#ifndef APIPELINE
#define APIPELINE

#include <cstdint>
#include <stack>

#include <amath.h>
#include <controller.h>

#include "rendersystem.h"

#include "resources/atexturegl.h"
#include "resources/amaterialgl.h"

class Engine;
class BaseController;

class ATextureGL;
class ACameraGL;
class ASpriteGL;

class ABlurGL;

class AAmbientOcclusionGL;

class APostProcessor;

class APipeline {
public:
    /*! \enum matrices_types */
    enum matrix_types {
        MATRIX_MODEL            = 0,
        MATRIX_VIEW,
        MATRIX_PROJECTION
    };

public:
    APipeline                   (Engine *engine);

    virtual ~APipeline          ();

    virtual void                draw                (Scene &scene, uint32_t);

    virtual ATextureGL         *postProcess         (ATextureGL &source);

    void                        setShaderParams     (uint32_t program);

    /*!
        Resizing current texture buffers.
        @param[in]  width       New screen width.
        @param[in]  height      New screen height.
    */
    virtual void                resize              (int32_t width, int32_t height);

    void                        clearScreen         (const ATextureGL &target);
    void                        drawScreen          (const ATextureGL &source, const ATextureGL &target);

    void                        drawQuad            ();

    void                        drawTexturedQuad    (const ATextureGL &texture);

    void                        drawComponents      (AObject &object, uint8_t layer);

    void                        setColor            (const Vector4 &color)  { m_Color = color; }
    void                        resetColor          ()                      { m_Color = Vector4(1.0f); }

    Vector2                     screen              () const                { return m_Screen; }
    Vector3                     world               () const                { return m_World; }

    Vector4                     idCode              (uint32_t id);

    AMaterialGL                *materialDirect      () const { return m_pDirect; }
    AMaterialGL                *materialPoint       () const { return m_pPoint; }
    AMaterialGL                *materialSpot        () const { return m_pSpot; }

    AMaterialGL                *materialFont        () const { return m_pFont; }

    ABlurGL                    *filterBlur          () { return m_pBlur; }

    uint32_t                    depthBuffer         () { return m_DepthBuffer; }
    ATextureGL                 &depthTexture        () { return m_Depth; }

    Vector4List                 screenVertices      () const { return m_Vertices; }
    Vector2List                 screenCoords        () const { return m_Coords; }

    void                        setTransform        (const Matrix4 &mat);
    void                        resetTransform      ();
    void                        setPos              (const Vector3 &pos, Matrix4 &m);
    void                        setScl              (const Vector3 &scl, Matrix4 &m);

    void                        makeOrtho           ();

    void                        cameraReset         ();
    ACameraGL                  *activeCamera        ();

    void                        overrideController  (IController *controller) { m_pController = controller; }

    Scene                      *scene               ();

    void                        loadMatrix          (matrix_types type, const Matrix4 &m);

protected:
    void                        cameraSet           (ACameraGL &camera);

    Matrix4                     modelView           () const { return m_View * m_Model; }
    Matrix4                     projection          () const { return m_Projection; }

    void                        initLightMaterial   (AMaterialGL *material);

    void                        updateLights        (AObject &object, uint8_t layer);
    void                        updateShadows       (AObject &object);

    void                        analizeScene        (AObject &object, IController *controller);
protected:
    ATextureGL                  m_Depth;
    /// Frame buffers
    uint32_t                    m_DepthBuffer;
    uint32_t                    m_ScreenBuffer;

    Scene                      *m_pScene;

    Vector2                     m_Screen;

    Vector3                     m_World;

    Vector4                     m_Color;

    Matrix4                     m_View;
    Matrix4                     m_Model;
    Matrix4                     m_Projection;

    Matrix4                     m_ModelView;
    Matrix4                     m_ModelViewProjectionInversed;

    stack<Matrix4>              m_MatrixStack;

    Engine                     *m_pEngine;

    ABlurGL                    *m_pBlur;

    AAmbientOcclusionGL        *m_pAO;

    list<APostProcessor *>      m_PostEffects;

    IController                *m_pController;

    ASpriteGL                  *m_pSprite;

    AMaterialGL                *m_pDirect;
    AMaterialGL                *m_pPoint;
    AMaterialGL                *m_pSpot;

    AMaterialGL                *m_pFont;

    Vector4List                 m_Vertices;
    Vector2List                 m_Coords;

    uint32_t                    m_VAO;
};

class IDrawObjectGL {
public:
    enum LayerTypes {
        DEFAULT     = (1<<0),
        RAYCAST     = (1<<1),
        SHADOWCAST  = (1<<2),
        LIGHT       = (1<<3),
        TRANSLUCENT = (1<<4),
        UI          = (1<<6)
    };
public:
    virtual void                draw                (APipeline &pipeline, int8_t layer) = 0;
};

#endif // APIPELINE
