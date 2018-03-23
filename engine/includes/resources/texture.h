#ifndef TEXTURE_H
#define TEXTURE_H

#include <amath.h>

#include "engine.h"

class NEXT_LIBRARY_EXPORT Texture : public Object {
    A_REGISTER(Texture, Object, Resources)

public:
    enum TextureType {
        Flat,
        Cubemap
    };

    enum FormatType {
        RED,
        RGB,
        RGBA,
        DXT1,
        DXT5,
        ETC2
    };

    enum FilteringType {
        None,
        Bilinear,
        Trilinear
    };

    enum WrapType {
        Clamp,
        Repeat,
        Mirrored
    };

public:
    Texture                     ();

    virtual ~Texture            ();

    uint32_t                    width                       () const;
    uint32_t                    height                      () const;

    inline bool                 isCompressed                () const { return (m_format == DXT1 || m_format == DXT5 || m_format == ETC2); }
    inline bool                 isCubemap                   () { return (m_type == Cubemap); }

    void                        loadUserData                (const VariantMap &data);

protected:
    virtual void                clear                       ();

public:
    uint32_t                    m_format;
    uint32_t                    m_components;
    TextureType                 m_type;
    FilteringType               m_filtering;
    WrapType                    m_wrap;

    uint32_t                    m_Width;
    uint32_t                    m_Height;

};

#endif // TEXTURE_H
