#ifndef SHADER
#define SHADER

#include "engine.h"
#include "texture.h"

class Material;

class NEXT_LIBRARY_EXPORT MaterialInstance {
public:
    MaterialInstance            (Material *material);

    ~MaterialInstance           ();

    Material                   *material            () const;

    const Texture              *texture             (const char *name);

    void                        setFloat            (const char *name, float value);
    void                        setVector2          (const char *name, const Vector2 &value);
    void                        setVector3          (const char *name, const Vector3 &value);
    void                        setVector4          (const char *name, const Vector4 &value);

    void                        setMatrix4          (const char *name, const Vector4 &value);

    void                        setTexture          (const char *name, const Texture *value);

protected:
    typedef map<string, const Texture *>            TextureMap;

    Material                   *m_pMaterial;

    TextureMap                  m_Textures;

    VariantMap                  m_Uniforms;
};

class NEXT_LIBRARY_EXPORT Material : public Object {
    A_REGISTER(Material, Object, Resources)
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

public:
    Material                    ();

    virtual ~Material           ();

    void                        loadUserData                (const VariantMap &data);

    MaterialType                materialType                () const;

    LightModelType              lightModel                  () const;

    BlendType                   blendMode                   () const;

    bool                        isDoubleSided               () const;
    void                        setDoubleSided              (bool flag);

    uint8_t                     surfaces                    () const;

    virtual MaterialInstance   *createInstance              ();

protected:
    virtual void                clear                       ();

protected:
    typedef map<string, const Texture *>    TextureMap;

    BlendType                   m_BlendMode;

    LightModelType              m_LightModel;

    MaterialType                m_MaterialType;

    bool                        m_DoubleSided;

    bool                        m_Tangent;

    TextureMap                  m_Textures;

    uint8_t                     m_Surfaces;

};
typedef vector<MaterialInstance*>   MaterialArray;

#endif // SHADER

