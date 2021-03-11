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

    typedef map<string, Texture *> TextureMap;
    typedef map<string, Variant> UniformMap;

public:
    Material();
    ~Material();

    virtual void clear();

    bool doubleSided() const;
    void setDoubleSided(bool flag);

    bool depthTest() const;
    void setDepthTest(bool test);

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
    int m_BlendMode;

    int m_LightModel;

    int m_MaterialType;

    bool m_DoubleSided;

    bool m_DepthTest;

    bool m_DepthWrite;

    TextureMap m_Textures;

    UniformMap m_Uniforms;

    int32_t m_Surfaces;

};

class NEXT_LIBRARY_EXPORT MaterialInstance {
public:
    struct Info {
        uint32_t type;

        int32_t count;

        void *ptr;
    };

    typedef unordered_map<string, Info> InfoMap;

public:
    MaterialInstance(Material *material);
    ~MaterialInstance();

    Material *material() const;

    Texture *texture(const char *name);

    InfoMap &params();

    void setInteger(const char *name, int32_t *value, int32_t count = 1);

    void setFloat(const char *name, float *value, int32_t count = 1);
    void setVector2(const char *name, Vector2 *value, int32_t count = 1);
    void setVector3(const char *name, Vector3 *value, int32_t count = 1);
    void setVector4(const char *name, Vector4 *value, int32_t count = 1);

    void setMatrix4(const char *name, Matrix4 *value, int32_t count = 1);

    void setTexture(const char *name, Texture *value, int32_t count = 1);

    uint16_t surfaceType() const;
    void setSurfaceType(uint16_t type);

protected:
    friend class Material;

    Material *m_pMaterial;

    InfoMap m_Info;

    uint16_t m_SurfaceType;
};

#endif // SHADER

