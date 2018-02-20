#ifndef APIPELINE
#define APIPELINE

#include <cstdint>
#include <stack>

#include <amath.h>
#include <controller.h>

#include "rendersystem.h"

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


class IDrawObjectGL {
public:
    virtual void                draw                (APipeline &pipeline, int8_t layer) = 0;
};

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

    virtual void                resize              (int32_t width, int32_t height);

    void                        drawScreen          (const ATextureGL &source, const ATextureGL &target);

    void                        drawMesh            (const Matrix4 &model, Mesh *mesh, uint32_t surface = 0, uint8_t layer = IRenderSystem::DEFAULT, MaterialInstance *material = 0);

    void                        drawQuad            (const Matrix4 &model, uint8_t layer, MaterialInstance *material = 0, const Texture *texture = 0);

    void                        drawComponents      (AObject &object, uint8_t layer);

    void                        setColor            (const Vector4 &color)  { m_Color = color; }
    void                        resetColor          ()                      { m_Color = Vector4(1.0f); }

    Vector2                     screen              () const                { return m_Screen; }
    Vector3                     world               () const                { return m_World; }

    Vector4                     idCode              (uint32_t id);

    AMeshGL                    *meshPlane           () const { return m_pPlane; }
    AMeshGL                    *meshCube            () const { return m_pCube; }

    ABlurGL                    *filterBlur          () { return m_pBlur; }

    uint32_t                    depthBuffer         () { return m_DepthBuffer; }
    ATextureGL                 &depthTexture        () { return m_Depth; }

    void                        makeOrtho           ();

    void                        cameraReset         ();
    ACameraGL                  *activeCamera        ();

    void                        overrideController  (IController *controller) { m_pController = controller; }

    void                        loadMatrix          (matrix_types type, const Matrix4 &m);

    void                        cameraSet           (const Camera &camera);

protected:
    Matrix4                     modelView           () const { return m_View * m_Model; }
    Matrix4                     projection          () const { return m_Projection; }

    void                        initLightMaterial   (AMaterialGL *material);

    void                        updateLights        (AObject &object, uint8_t layer);
    void                        updateShadows       (AObject &object);

    void                        analizeScene        (AObject &object, IController *controller);

    void                        uploadMesh          (Mesh *mesh);

    void                        clearMesh           (const Mesh *mesh);

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

    Engine                     *m_pEngine;

    ABlurGL                    *m_pBlur;

    AAmbientOcclusionGL        *m_pAO;

    list<APostProcessor *>      m_PostEffects;

    IController                *m_pController;

    AMaterialGL                *m_pSprite;
    AMaterialGL                *m_pMesh;

    MaterialInstance           *m_pSpriteInstance;
    MaterialInstance           *m_pMeshInstance;

    AMeshGL                    *m_pPlane;
    AMeshGL                    *m_pCube;

    typedef unordered_map<const void *, AMeshGL::BufferVector> VAOMap;

    VAOMap                      m_vaoMap;
};

#endif // APIPELINE
