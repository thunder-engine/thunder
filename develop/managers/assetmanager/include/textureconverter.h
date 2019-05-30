#ifndef TEXTURECONVERTER_H
#define TEXTURECONVERTER_H

#include "resources/texture.h"

#include "converters/converter.h"

class QImage;

class TextureImportSettings : public IConverterSettings {
    Q_OBJECT

    Q_PROPERTY(TextureType Type READ textureType WRITE setTextureType DESIGNABLE true USER true)
    Q_PROPERTY(FormatType Format READ formatType WRITE setFormatType DESIGNABLE true USER true)
    Q_PROPERTY(WrapType Wrap READ wrap WRITE setWrap DESIGNABLE true USER true)
    Q_PROPERTY(bool MIP_maping READ lod WRITE setLod DESIGNABLE true USER true)
    Q_PROPERTY(FilteringType Filtering READ filtering WRITE setFiltering DESIGNABLE true USER true)

public:
    enum FormatType {
        Uncompressed_R8G8B8     = Texture::RGB8,
        Uncompressed_R8G8B8A8   = Texture::RGBA8,
    };

    enum TextureType {
        Texture2D   = Texture::Flat,
        Cubemap     = Texture::Cubemap
    };

    enum FilteringType {
        None        = Texture::None,
        Bilinear    = Texture::Bilinear,
        Trilinear   = Texture::Trilinear
    };

    enum WrapType {
        Clamp,
        Repeat,
        Mirrored
    };

    Q_ENUM(WrapType)
    Q_ENUM(FilteringType)
    Q_ENUM(TextureType)
    Q_ENUM(FormatType)

public:
    TextureImportSettings       ();

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

protected:
    TextureType                 m_TextureType;

    FormatType                  m_FormType;

    FilteringType               m_Filtering;

    WrapType                    m_Wrap;

    bool                        m_Lod;
};

class TextureSerial : public Texture {
public:
    VariantList                 m_Surfaces;

    void                        loadUserData                (const VariantMap &data);

    VariantMap                  saveUserData                () const;
protected:
    friend class TextureConverter;

};

class TextureConverter : public IConverter {
public:
    QStringList suffixes() const { return {"bmp", "dds", "jpg", "jpeg", "png", "tga", "ico", "tif"}; }
    uint32_t                    contentType                 () const { return ContentTexture; }
    uint32_t                    type                        () const { return MetaType::type<Texture *>(); }
    uint8_t                     convertFile                 (IConverterSettings *);

    VariantMap                  convertResource             (IConverterSettings *);

    IConverterSettings         *createSettings              () const;

protected:
    bool                        tgaReader                   (IConverterSettings &settings, QImage &t);
};

#endif // TEXTURECONVERTER_H
