#ifndef COMMANDBUFFER_H
#define COMMANDBUFFER_H

#include "engine.h"

#define GLOBAL_BIND     0
#define LOCAL_BIND      1
#define UNIFORM_BIND    4

class RenderTarget;
class Texture;
class Mesh;

class Camera;
class MaterialInstance;

struct Global {
    Matrix4 view;
    Matrix4 projection;
    Matrix4 cameraView;
    Matrix4 cameraProjection;
    Matrix4 cameraProjectionInv;
    Matrix4 cameraScreenToWorld;
    Matrix4 cameraWorldToScreen;

    Vector4 cameraPosition;
    Vector4 cameraTarget;
    Vector4 cameraScreen;
    Vector4 lightPageSize;

    Vector4 lightAmbient;
    float clip;
    float time;
    float padding[10];
};

struct Local {
    Matrix4 model;

    Vector4 color;
    float padding[12];
};

class ENGINE_EXPORT CommandBuffer: public Object {
    A_REGISTER(CommandBuffer, Object, System)

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
    CommandBuffer();

    virtual void clearRenderTarget(bool clearColor = true, const Vector4 &color = Vector4(0.0f), bool clearDepth = true, float depth = 1.0f);

    virtual void drawMesh(const Matrix4 &model, Mesh *mesh, uint32_t sub, uint32_t layer = CommandBuffer::DEFAULT, MaterialInstance *material = nullptr);

    virtual void drawMeshInstanced(const Matrix4 *models, uint32_t count, Mesh *mesh, uint32_t sub, uint32_t layer = CommandBuffer::DEFAULT, MaterialInstance *material = nullptr);

    virtual void setRenderTarget(RenderTarget *target, uint32_t level = 0);

    virtual void setColor(const Vector4 &color);

    virtual void setScreenProjection(float x = -0.5f, float y = -0.5f, float width = 0.5f, float height = 0.5f);

    virtual void resetViewProjection();

    virtual void setViewProjection(const Matrix4 &view, const Matrix4 &projection);

    virtual void setGlobalValue(const char *name, const Variant &value);

    virtual void setGlobalTexture(const char *name, Texture *value);

    virtual void setViewport(int32_t x, int32_t y, int32_t width, int32_t height);

    virtual void enableScissor(int32_t x, int32_t y, int32_t width, int32_t height);

    virtual void disableScissor();

    virtual Matrix4 projection() const;

    virtual Matrix4 view() const;

    virtual Texture *texture(const char *name) const;

    virtual void finish();

    Vector2 viewport() const;

    static Vector4 idToColor(uint32_t id);

    static bool isInited();

    static void setInited();

protected:
    Global m_global;
    Local m_local;

    Matrix4 m_saveView;
    Matrix4 m_saveProjection;

    int32_t m_viewportX;
    int32_t m_viewportY;
    int32_t m_viewportWidth;
    int32_t m_viewportHeight;

};

#endif // COMMANDBUFFER_H
