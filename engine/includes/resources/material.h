#ifndef MATERIAL_H
#define MATERIAL_H

#include "engine.h"
#include "texture.h"

class Transform;
class MaterialInstance;
class CommandBuffer;

class ENGINE_EXPORT Material : public Resource {
    A_OBJECT(Material, Resource, Resources)

    A_PROPERTIES(
        A_PROPERTY(int, materialType, Material::materialType, Material::setMaterialType),
        A_PROPERTY(int, lightModel, Material::lightModel, Material::setLightModel),
        A_PROPERTY(bool, doubleSided, Material::doubleSided, Material::setDoubleSided),
        A_PROPERTY(bool, wireframe, Material::wireframe, Material::setWireframe)
    )
    A_NOMETHODS()
    A_ENUMS(
        A_ENUM(Type,
               A_VALUE(Surface),
               A_VALUE(PostProcess),
               A_VALUE(LightFunction)),

        A_ENUM(LightModel,
               A_VALUE(Unlit),
               A_VALUE(Lit),
               A_VALUE(Subsurface))
    )

public:
    enum Type {
        Surface,
        PostProcess,
        LightFunction
    };

    enum LightModel {
        Unlit,
        Lit,
        Subsurface
    };

    enum SurfaceType {
        Static,
        Skinned,
        Billboard
    };

    enum BlendOp {
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
        int32_t alphaOperation = BlendOp::Add;

        int32_t colorOperation = BlendOp::Add;

        int32_t destinationAlphaBlendMode = BlendFactor::One;

        int32_t destinationColorBlendMode = BlendFactor::One;

        int32_t sourceAlphaBlendMode = BlendFactor::Zero;

        int32_t sourceColorBlendMode = BlendFactor::Zero;

        bool enabled = false;

    };

    struct DepthState {
        int32_t compareFunction = TestFunction::Less;

        bool writeEnabled = true;

        bool enabled = false;

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
        std::string name;

        Texture *texture;

        int32_t binding;

        int32_t flags;
    };
    typedef std::list<TextureItem> Textures;

public:
    Material();
    ~Material();

    bool doubleSided() const;
    void setDoubleSided(bool flag);

    int materialType() const;
    void setMaterialType(int type);

    int lightModel() const;
    void setLightModel(int model);

    bool wireframe() const;
    void setWireframe(bool wireframe);

    virtual MaterialInstance *createInstance(SurfaceType type = SurfaceType::Static);

    void loadUserData(const VariantMap &data) override;

    void initInstance(MaterialInstance *instance);

protected:
    void switchState(Resource::State state) override;
    bool isUnloadable() override;

private:
    void loadBlendState(const VariantList &data);
    void loadDepthState(const VariantList &data);
    void loadStencilState(const VariantList &data);

protected:
    struct UniformItem {
        std::string name;

        Variant defaultValue;

        size_t size;

        size_t offset;
    };
    typedef std::vector<UniformItem> Uniforms;

protected:
    friend class MaterialInstance;

    Textures m_textures;

    Uniforms m_uniforms;

    BlendState m_blendState;

    DepthState m_depthState;

    StencilState m_stencilState;

    uint32_t m_uniformSize;

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

    uint32_t instanceCount() const;
    void setInstanceCount(uint32_t number);

    uint32_t instanceSize() const;

    void setBool(const char *name, const bool *value, int32_t count = 1);

    void setInteger(const char *name, const int32_t *value, int32_t count = 1);

    void setFloat(const char *name, const float *value, int32_t count = 1);
    void setVector2(const char *name, const Vector2 *value, int32_t count = 1);
    void setVector3(const char *name, const Vector3 *value, int32_t count = 1);
    void setVector4(const char *name, const Vector4 *value, int32_t count = 1);

    void setMatrix4(const char *name, const Matrix4 *value, int32_t count = 1);

    void setTexture(const char *name, Texture *texture);

    Transform *transform();
    void setTransform(Transform *transform);

    void setTransform(const Matrix4 &transform);

    uint32_t paramCount() const;
    std::string paramName(uint32_t index) const;
    Variant paramValue(uint32_t index) const;

    uint16_t surfaceType() const;
    void setSurfaceType(uint16_t type);

    ByteArray &rawUniformBuffer();

    void batch(MaterialInstance &instance);
    void resetBatches();

    int hash() const;

protected:
    void setBufferValue(const char *name, const void *value);

    Texture *texture(CommandBuffer &buffer, int32_t binding);

    virtual void overrideTexture(int32_t binding, Texture *texture);

protected:
    friend class Material;

    std::unordered_map<int32_t, Texture *> m_textureOverride;
    std::map<std::string, Variant> m_paramOverride;

    ByteArray m_uniformBuffer;
    ByteArray m_batchBuffer;

    Material *m_material;

    Transform *m_transform;

    uint32_t m_instanceCount;
    uint32_t m_batchesCount;

    uint32_t m_hash;
    uint32_t m_transformHash;

    uint16_t m_surfaceType;

};

#endif // MATERIAL_H

