#ifndef SHADER
#define SHADER

#include "engine.h"
#include "texture.h"

class Material;

class NEXT_LIBRARY_EXPORT MaterialInstance {
public:
    MaterialInstance            (Material *material);

    Material                   *material            () const;

    virtual void                setFloat            (const char *name, float value);
    virtual void                setVector2          (const char *name, const Vector2 &value);
    virtual void                setVector3          (const char *name, const Vector3 &value);
    virtual void                setVector4          (const char *name, const Vector4 &value);

    virtual void                setMatrix4          (const char *name, const Vector4 &value);
protected:
    Material                   *m_pMaterial;
};

class NEXT_LIBRARY_EXPORT Material : public AObject {
    A_REGISTER(Material, AObject, Resources)
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

    void                        loadUserData                (const AVariantMap &data);

    MaterialType                materialType                () const;

    LightModelType              lightModel                  () const;

    BlendType                   blendMode                   () const;

    bool                        isDoubleSided               () const;
    void                        setDoubleSided              (bool flag);

    uint8_t                     surfaces                    () const;

    bool                        overrideTexture             (const string &name, const Texture *texture);

    const Texture              *texture                     (const string &name);

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

