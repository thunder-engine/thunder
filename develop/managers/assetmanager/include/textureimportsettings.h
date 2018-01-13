#ifndef TEXTUREIMPORTSETTINGS_H
#define TEXTUREIMPORTSETTINGS_H

#include "baseconvertersettings.h"

#include <resources/texture.h>

class TextureImportSettings : public BaseConverterSettings {
    Q_OBJECT

    Q_PROPERTY(TextureType Type READ textureType WRITE setTextureType DESIGNABLE true USER true)
    Q_ENUMS(TextureType)
    Q_PROPERTY(FormatType Format READ formatType WRITE setFormatType DESIGNABLE true USER true)
    Q_ENUMS(FormatType)
    Q_PROPERTY(WrapType Wrap READ wrap WRITE setWrap DESIGNABLE true USER true)
    Q_ENUMS(WrapType)
    Q_PROPERTY(bool MIP_maping READ lod WRITE setLod DESIGNABLE true USER true)
    Q_PROPERTY(FilteringType Filtering READ filtering WRITE setFiltering DESIGNABLE true USER true)
    Q_ENUMS(FilteringType)

public:
    enum FormatType {
        Uncompressed_R8G8B8     = Texture::RGB,
        Uncompressed_R8G8B8A8   = Texture::RGBA,
    };

    enum TextureType {
        Texture2D   = Texture::Flat,
        Cubemap     = Texture::Cubemap
    };

    enum FilteringType {
        None,
        Bilinear
    };

    enum WrapType {
        Clamp,
        Repeat,
        Mirrored
    };

public:
    TextureImportSettings       (QObject *parent = 0);

    TextureType                 textureType                 () const;
    void                        setTextureType              (TextureType type);

    FormatType                  formatType                  () const;
    void                        setFormatType               (FormatType type);

    FilteringType               filtering                   () const;
    void                        setFiltering                (FilteringType type);

    WrapType                    wrap                        () const;
    void                        setWrap                     (WrapType wrap);

    bool                        lod                         () const;
    void                        setLod                      (bool lod);

    void                        loadProperties              (const QVariantMap &map);

protected:
    TextureType                 m_TextureType;

    FormatType                  m_FormType;

    FilteringType               m_Filtering;

    WrapType                    m_Wrap;

    bool                        m_Lod;
};

#endif // TEXTUREIMPORTSETTINGS_H
