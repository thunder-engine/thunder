#ifndef MATERIAL_H
#define MATERIAL_H

#include "engine.h"
#include "texture.h"

class Material;

class MaterialInstance;

class ENGINE_EXPORT Material : public Resource {
    A_REGISTER(Material, Resource, Resources)

    A_PROPERTIES(
        A_PROPERTY(bool, doubleSided, Material::doubleSided, Material::setDoubleSided),
        A_PROPERTY(int, materialType, Material::materialType, Material::setMaterialType),
        A_PROPERTY(int, lightModel, Material::lightModel, Material::setLightModel),
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
               A_VALUE(Translucent))
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
        Static,
        Skinned,
        Billboard,
        Oriented,
        Fullscreen
    };

    enum BlendMode {
        Add,
        Subtract,
        ReverseSubtract,
        Min,
        Max
    };

    enum BlendFactor {
        Zero,
        One,
        SourceColor,
        OneMinusSourceColor,
        DestinationColor,
        OneMinusDestinationColor,
        SourceAlpha,
        OneMinusSourceAlpha,
        DestinationAlpha,
        OneMinusDestinationAlpha,
        SourceAlphaSaturate,
        ConstantColor,
        OneMinusConstantColor,
        ConstantAlpha,
        OneMinusConstantAlpha
    };

    enum ActionType {
        Keep,
        Clear,
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

    enum CullingMode {
        Front,
        Back,
        FrontAndBack
    };

    struct RasterState {
        int32_t cullingMode = CullingMode::Back;

        int32_t offsetUnits = 0;

        float offsetFactor = 0.0f;

        bool depthClip = false;

        bool enabled = true;

    };

    struct BlendState {
        int32_t alphaOperation = BlendMode::Add;

        int32_t colorOperation = BlendMode::Add;

        int32_t destinationAlphaBlendMode = BlendFactor::One;

        int32_t destinationColorBlendMode = BlendFactor::One;

        int32_t sourceAlphaBlendMode = BlendFactor::Zero;

        int32_t sourceColorBlendMode = BlendFactor::Zero;

        bool enabled = false;

    };

    struct DepthState {
        int32_t compareFunction = TestFunction::Less;

        bool writeEnabled = true;

        bool enabled = true;

    };

    struct StencilState {
        int32_t compareFunctionBack = TestFunction::Always;

        int32_t compareFunctionFront = TestFunction::Always;

        int32_t failOperationBack = ActionType::Keep;

        int32_t failOperationFront = ActionType::Keep;

        int32_t passOperationBack = ActionType::Keep;

        int32_t passOperationFront = ActionType::Keep;

        int32_t zFailOperationBack = ActionType::Keep;

        int32_t zFailOperationFront = ActionType::Keep;

        int32_t readMask = 1;

        int32_t writeMask = 1;

        int32_t reference = 0;

        bool enabled = false;

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

    int materialType() const;
    void setMaterialType(int type);

    int lightModel() const;
    void setLightModel(int model);

    void setTexture(const string &name, Texture *texture);

    bool wireframe() const;
    void setWireframe(bool wireframe);

    virtual MaterialInstance *createInstance(SurfaceType type = SurfaceType::Static);

    void loadUserData(const VariantMap &data) override;

protected:
    void initInstance(MaterialInstance *instance);

    void switchState(ResourceState state) override;
    bool isUnloadable() override;

private:
    void loadBlendState(const VariantList &data);
    void loadDepthState(const VariantList &data);
    void loadStencilState(const VariantList &data);

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

    BlendState m_blendState;

    DepthState m_depthState;

    StencilState m_stencilState;

    uint32_t m_uniformSize;

    int32_t m_vertexBits;

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

