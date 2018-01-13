#ifndef SHADER
#define SHADER

#include "engine.h"
#include "texture.h"

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

    bool                        overrideTexture             (const string &name, Texture *texture);

    Texture                    *texture                     (const string &name);

protected:
    virtual void                clear                       ();

protected:
    typedef map<string, Texture *>  TextureMap;

    BlendType                   m_BlendMode;

    LightModelType              m_LightModel;

    MaterialType                m_MaterialType;

    bool                        m_DoubleSided;

    bool                        m_Tangent;

    TextureMap                  m_Textures;

    uint8_t                     m_Surfaces;

};
typedef vector<Material*>       MaterialArray;

#endif // SHADER

