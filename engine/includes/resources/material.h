#ifndef SHADER
#define SHADER

#include "engine.h"
#include "texture.h"

class Material;

class NEXT_LIBRARY_EXPORT MaterialInstance {
public:
    struct Info {
        uint32_t type;

        uint32_t count;

        void *ptr;
    };

    typedef unordered_map<string, Info>             InfoMap;

public:
    MaterialInstance            (Material *material);

    ~MaterialInstance           ();

    Material                   *material            () const;

    Texture                    *texture             (const char *name);

    InfoMap                    &params              ();

    void                        setInteger          (const char *name, int32_t *value, uint32_t count = 1);

    void                        setFloat            (const char *name, float *value, uint32_t count = 1);
    void                        setVector2          (const char *name, Vector2 *value, uint32_t count = 1);
    void                        setVector3          (const char *name, Vector3 *value, uint32_t count = 1);
    void                        setVector4          (const char *name, Vector4 *value, uint32_t count = 1);

    void                        setMatrix4          (const char *name, Matrix4 *value, uint32_t count = 1);

    void                        setTexture          (const char *name, Texture *value, uint32_t count = 1);

protected:
    Material                   *m_pMaterial;

    InfoMap                     m_Info;
};

class NEXT_LIBRARY_EXPORT Material : public Resource {
    A_REGISTER(Material, Resource, Resources)

public:
    enum MaterialType {
        Surface,
        PostProcess,
        LightFunction
    };

    /*! \enum LightModelType */
    enum LightModelType {
        Unlit,
        Lit,
        Subsurface
    };

    /*! \enum BlendType */
    enum BlendType {
        Opaque,
        Additive,
        Translucent
    };

    enum SurfaceType {
        Static                  = (1<<0),
        Skinned                 = (1<<1),
        Billboard               = (1<<2),
        Oriented                = (1<<3)
    };

    typedef map<string, Texture *> TextureMap;

public:
    Material ();
    ~Material ();

    virtual void                clear                       ();

    void                        loadUserData                (const VariantMap &data) override;

    MaterialType                materialType                () const;

    LightModelType              lightModel                  () const;

    BlendType                   blendMode                   () const;

    bool                        isDoubleSided               () const;
    void                        setDoubleSided              (bool flag);

    bool                        isDepthTest                 () const;
    void                        setDepthTest                (bool flag);

    void                        setTexture                  (const string &name, Texture *texture);

    int32_t                     surfaces                    () const;

    virtual MaterialInstance   *createInstance              ();

protected:
    BlendType                   m_BlendMode;

    LightModelType              m_LightModel;

    MaterialType                m_MaterialType;

    bool                        m_DoubleSided;

    bool                        m_DepthTest;

    TextureMap                  m_Textures;

    int32_t                     m_Surfaces;

};

#endif // SHADER

