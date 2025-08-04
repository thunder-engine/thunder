#ifndef COMMANDBUFFER_H
#define COMMANDBUFFER_H

#include "engine.h"

#include "material.h"

#define GLOBAL_BIND     0
#define LOCAL_BIND      1
#define SKIN_BIND       2
#define UNIFORM_BIND    4

class ComputeInstance;
class RenderTarget;
class Texture;
class Mesh;

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
    Vector4 shadowPageSize;

    float clip;
    float time;
    float deltaTime;
    float padding[13];
};

class ENGINE_EXPORT CommandBuffer: public Object {
    A_OBJECT(CommandBuffer, Object, System)

public:
    CommandBuffer();

    virtual void dispatchCompute(ComputeInstance *shader, int32_t groupsX, int32_t groupsY, int32_t groupsZ);

    virtual void drawMesh(Mesh *mesh, uint32_t sub, uint32_t layer, MaterialInstance &instance);

    virtual void setRenderTarget(RenderTarget *target, uint32_t level = 0);

    virtual void setViewProjection(const Matrix4 &view, const Matrix4 &projection);

    virtual void setGlobalValue(const char *name, const Variant &value);

    virtual void setGlobalTexture(const TString &name, Texture *texture);

    virtual void setViewport(int32_t x, int32_t y, int32_t width, int32_t height);

    virtual void enableScissor(int32_t x, int32_t y, int32_t width, int32_t height);

    virtual void disableScissor();

    virtual Matrix4 projection() const;

    virtual Matrix4 view() const;

    virtual Texture *texture(const TString &name) const;

    virtual void beginDebugMarker(const char *name);
    virtual void endDebugMarker();

    Vector2 viewport() const;

    static Vector4 idToColor(uint32_t id);

    static bool isInited();

    static void setInited();

protected:
    Global m_global;

    Material::Textures m_textures;

    int32_t m_viewportX;
    int32_t m_viewportY;
    int32_t m_viewportWidth;
    int32_t m_viewportHeight;

};

#endif // COMMANDBUFFER_H
