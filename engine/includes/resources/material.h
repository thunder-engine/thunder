#ifndef MATERIAL_H
#define MATERIAL_H

#include "engine.h"
#include "texture.h"

class Material;

class MaterialInstance;

struct RasterState {
    int32_t cullingMode;

    int32_t offsetUnits;

    float offsetFactor = 0.0f;

    bool depthClip = false;

    bool enabled = true;

};

struct BlendState {
    int32_t alphaOperation;

    int32_t colorOperation;

    int32_t destinationAlphaBlendMode;

    int32_t destinationColorBlendMode;

    int32_t sourceAlphaBlendMode;

    int32_t sourceColorBlendMode;

    int32_t writeMask;

    bool enabled = false;

};

struct DepthState {
    int32_t compareFunction = 1; // Less

    bool enabled = true;

    bool writeEnabled = true;

};

struct StencilState {
    int32_t compareFunctionBack = 7; // Always

    int32_t compareFunctionFront = 7; // Always

    int32_t failOperationBack = 0; // Keep

    int32_t failOperationFront = 0; // Keep

    int32_t passOperationBack = 0; // Keep

    int32_t passOperationFront = 0; // Keep

    int32_t readMask = 1;

    int32_t writeMask = 1;

    int32_t reference = 0;

    int32_t zFailOperationBack = 0; // Keep

    int32_t zFailOperationFront = 0; // Keep

    bool enabled = false;

};

class ENGINE_EXPORT Material : public Resource {
    A_REGISTER(Material, Resource, Resources)

    A_PROPERTIES(
        A_PROPERTY(bool, doubleSided, Material::doubleSided, Material::setDoubleSided),
        A_PROPERTY(bool, depthTest, Material::depthTest, Material::setDepthTest),
        A_PROPERTY(bool, depthWrite, Material::depthWrite, Material::setDepthWrite),
        A_PROPERTY(int, materialType, Material::materialType, Material::setMaterialType),
        A_PROPERTY(int, lightModel, Material::lightModel, Material::setLightModel),
        A_PROPERTY(int, blendMode, Material::blendMode, Material::setBlendMode),
        A_PROPERTY(bool, wireframe, Material::wireframe, Material::setWireframe)
    )
    A_METHODS(
        A_METHOD(void, Material::setTexture)
    )
    A_ENUMS(
        A_ENUM(MaterialType,
               A_VALUE(Surface),
               A_VALUE(PostProcess),
               A_VALUE(LightFunction)),

        A_ENUM(LightModelType,
               A_VALUE(Unlit),
               A_VALUE(Lit),
               A_VALUE(Subsurface)),

        A_ENUM(BlendType,
               A_VALUE(Opaque),
               A_VALUE(Additive),
               A_VALUE(Translucent)),

        A_ENUM(SurfaceType,
               A_VALUE(Static),
               A_VALUE(Skinned),
               A_VALUE(Billboard),
               A_VALUE(Oriented))
    )

public:
    enum MaterialType {
        Surface,
        PostProcess,
        LightFunction
    };

    enum LightModelType {
        Unlit,
        Lit,
        Subsurface
    };

    enum BlendType {
        Opaque,
        Additive,
        Translucent
    };

    enum SurfaceType {
        Static    = (1<<0),
        Skinned   = (1<<1),
        Billboard = (1<<2),
        Oriented  = (1<<3),
        Fullscreen= (1<<4)
    };

    enum ActionType {
        Keep,
        Zero,
        Replace,
        Increment,
        IncrementWrap,
        Decrement,
        DecrementWrap,
        Invert
    };

    enum TestFunction {
        Never,
        Less,
        LessOrEqual,
        Greater,
        GreaterOrEqual,
        Equal,
        NotEqual,
        Always
    };

    struct TextureItem {
        string name;

        Texture *texture;

        int32_t binding;

        int32_t flags;
    };
    typedef list<TextureItem> Textures;

public:
    Material();
    ~Material();

    bool doubleSided() const;
    void setDoubleSided(bool flag);

    bool depthTest() const;
    void setDepthTest(bool test);

    bool depthWrite() const;
    void setDepthWrite(bool depth);

    int materialType() const;
    void setMaterialType(int type);

    int lightModel() const;
    void setLightModel(int model);

    int blendMode() const;
    void setBlendMode(int mode);

    void setTexture(const string &name, Texture *texture);

    bool wireframe() const;
    void setWireframe(bool wireframe);

    virtual MaterialInstance *createInstance(SurfaceType type = SurfaceType::Static);

    void loadUserData(const VariantMap &data) override;

protected:
    void initInstance(MaterialInstance *instance);

    void switchState(ResourceState state) override;
    bool isUnloadable() override;

protected:
    struct UniformItem {
        string name;

        Variant defaultValue;

        size_t size;

        size_t offset;
    };
    typedef vector<UniformItem> Uniforms;

    struct Attribute {
        uint32_t format;

        uint32_t location;
    };
    typedef vector<Attribute> Attributes;

protected:
    friend class MaterialInstance;

    Textures m_textures;

    Uniforms m_uniforms;

    Attributes m_attributes;

    RasterState m_rasterState;

    BlendState m_blendState;

    DepthState m_depthState;

    StencilState m_stencilState;

    uint32_t m_uniformSize;

    int32_t m_surfaces;

    int32_t m_blendMode;

    int32_t m_lightModel;

    int32_t m_materialType;

    bool m_doubleSided;

    bool m_wireframe;
};

class ENGINE_EXPORT MaterialInstance {
public:
    explicit MaterialInstance(Material *material);
    virtual ~MaterialInstance();

    Material *material() const;

    Texture *texture(const char *name);

    void setBool(const char *name, const bool *value, int32_t count = 1);

    void setInteger(const char *name, const int32_t *value, int32_t count = 1);

    void setFloat(const char *name, const float *value, int32_t count = 1);
    void setVector2(const char *name, const Vector2 *value, int32_t count = 1);
    void setVector3(const char *name, const Vector3 *value, int32_t count = 1);
    void setVector4(const char *name, const Vector4 *value, int32_t count = 1);

    void setMatrix4(const char *name, const Matrix4 *value, int32_t count = 1);

    virtual void setTexture(const char *name, Texture *texture);

    uint32_t paramCount() const;
    string paramName(uint32_t index) const;
    Variant paramValue(uint32_t index) const;

    uint16_t surfaceType() const;
    void setSurfaceType(uint16_t type);

protected:
    void setBufferValue(const char *name, const void *value);

protected:
    friend class Material;

    map<string, Texture *> m_textureOverride;
    map<string, Variant> m_paramOverride;

    Material *m_material;

    uint8_t *m_uniformBuffer;

    uint16_t m_surfaceType;

    bool m_uniformDirty;

};

#endif // MATERIAL_H

