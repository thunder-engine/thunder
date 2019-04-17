#ifndef TEXTURECONVERTER_H
#define TEXTURECONVERTER_H

#include "resources/texture.h"

#include "converters/converter.h"

class QImage;

class TextureImportSettings : public QObject, public IConverterSettings {
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

signals:
    void                        updated                     ();

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
    TextureConverter            () {}

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
