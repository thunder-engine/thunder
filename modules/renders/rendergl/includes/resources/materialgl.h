#ifndef MATERIALGL_H
#define MATERIALGL_H

#include <unordered_map>
#include <list>

#include <resources/material.h>

#include <engine.h>

#define GLOBAL_BIND     0
#define LOCAL_BIND      1

#define LIGHT_BIND      3
#define UNIFORM_BIND    4

class CommandBufferGL;

struct GlobalBufferObject {
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
};

struct LocalBufferObject {
    Matrix4 model;
    Vector4 color;
};

class MaterialInstanceGL : public MaterialInstance {
public:
    MaterialInstanceGL(Material *material);

    ~MaterialInstanceGL();

    bool bind(CommandBufferGL *buffer, uint32_t layer);

private:
    void setInteger(const char *name, const int32_t *value, int32_t count) override;

    void setFloat(const char *name, const float *value, int32_t count) override;

    void setVector2(const char *name, const Vector2 *value, int32_t count) override;

    void setVector3(const char *name, const Vector3 *value, int32_t count) override;

    void setVector4(const char *name, const Vector4 *value, int32_t count) override;

    void setMatrix4(const char *name, const Matrix4 *value, int32_t count) override;

    void setValue(const char *name, const void *value);

private:
    uint32_t m_instanceUbo;

    uint8_t *m_uniformBuffer;

    bool m_uniformDirty;
};

class MaterialGL : public Material {
    A_OVERRIDE(MaterialGL, Material, Resources)

    A_NOPROPERTIES()
    A_NOMETHODS()

    enum ShaderType {
        Static      = 1,
        Instanced,
        Skinned,
        Particle,
        LastVertex,

        Default     = 20,
        Simple,
        LastFragment
    };

    typedef unordered_map<uint32_t, uint32_t> ObjectMap;

public:
    MaterialGL();

    void loadUserData(const VariantMap &data) override;

    uint32_t bind(uint32_t layer, uint16_t vertex);

    uint32_t getProgram(uint16_t type);

    TextureList textures() const { return m_Textures; }

    uint32_t uniformSize() const;

protected:
    void switchState(ResourceState state) override;

    uint32_t buildShader(uint16_t type, const string &src = string());

    uint32_t buildProgram(uint32_t vertex, uint32_t fragment);

    bool checkShader(uint32_t shader, const string &path, bool link = false);

    MaterialInstance *createInstance(SurfaceType type = SurfaceType::Static) override;

private:
    friend class MaterialInstanceGL;

    ObjectMap m_Programs;

    map<uint16_t, string> m_ShaderSources;

    uint32_t m_uniformSize;

};

#endif // MATERIALGL_H
