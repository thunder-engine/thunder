#ifndef SHADER
#define SHADER

#include "engine.h"
#include "texture.h"

class Material;

class MaterialInstance;

class NEXT_LIBRARY_EXPORT Material : public Resource {
    A_REGISTER(Material, Resource, Resources)

    A_PROPERTIES(
        A_PROPERTY(bool, doubleSided, Material::doubleSided, Material::setDoubleSided),
        A_PROPERTY(bool, depthTest, Material::depthTest, Material::setDepthTest),
        A_PROPERTY(bool, depthWrite, Material::depthWrite, Material::setDepthWrite),
        A_PROPERTY(int, materialType, Material::materialType, Material::setMaterialType),
        A_PROPERTY(int, lightModel, Material::lightModel, Material::setLightModel),
        A_PROPERTY(int, blendMode, Material::blendMode, Material::setBlendMode)
    )
    A_METHODS(
        A_METHOD(void, Material::clear),
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
        Oriented  = (1<<3)
    };

    struct TextureItem {
        string name;

        Texture *texture;

        int32_t binding;

        int32_t flags;
    };

    struct UniformItem {
        string name;

        Variant value;

        size_t size;

        size_t offset;
    };

    typedef list<TextureItem> TextureList;
    typedef list<UniformItem> UniformList;

public:
    Material();
    ~Material();

    virtual void clear();

    bool doubleSided() const;
    void setDoubleSided(bool flag);

    bool depthTest() const;
    void setDepthTest(bool test);

    bool depthWrite() const;
    void setDepthWrite(bool write);

    int materialType() const;
    void setMaterialType(int type);

    int lightModel() const;
    void setLightModel(int model);

    int blendMode() const;
    void setBlendMode(int mode);

    void setTexture(const string &name, Texture *texture);

    virtual MaterialInstance *createInstance(SurfaceType type = SurfaceType::Static);

    void loadUserData(const VariantMap &data) override;

protected:
    void initInstance(MaterialInstance *instance);

protected:
    int m_BlendMode;

    int m_LightModel;

    int m_MaterialType;

    bool m_DoubleSided;

    bool m_DepthTest;

    bool m_DepthWrite;

    TextureList m_Textures;

    UniformList m_Uniforms;

    int32_t m_Surfaces;

};

class NEXT_LIBRARY_EXPORT MaterialInstance {
public:
    explicit MaterialInstance(Material *material);
    virtual ~MaterialInstance();

    Material *material() const;

    Texture *texture(const char *name);

    virtual void setInteger(const char *name, const int32_t *value, int32_t count = 1);

    virtual void setFloat(const char *name, const float *value, int32_t count = 1);
    virtual void setVector2(const char *name, const Vector2 *value, int32_t count = 1);
    virtual void setVector3(const char *name, const Vector3 *value, int32_t count = 1);
    virtual void setVector4(const char *name, const Vector4 *value, int32_t count = 1);

    virtual void setMatrix4(const char *name, const Matrix4 *value, int32_t count = 1);

    virtual void setTexture(const char *name, Texture *value);

    uint16_t surfaceType() const;
    void setSurfaceType(uint16_t type);

protected:
    friend class Material;

    Material *m_material;

    uint16_t m_surfaceType;

    map<string, Texture *> m_textureOverride;
};

#endif // SHADER

