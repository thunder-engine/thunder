#ifndef TEXTURECONVERTER_H
#define TEXTURECONVERTER_H

#include <resources/texture.h>
#include <resources/sprite.h>

#include <editor/converter.h>

class QImage;

class TextureImportSettings : public IConverterSettings {
    Q_OBJECT

    Q_PROPERTY(TextureType Type READ textureType WRITE setTextureType DESIGNABLE true USER true)
    Q_PROPERTY(FormatType Format READ formatType WRITE setFormatType DESIGNABLE true USER true)
    Q_PROPERTY(WrapType Wrap READ wrap WRITE setWrap DESIGNABLE true USER true)
    Q_PROPERTY(bool MIP_maping READ lod WRITE setLod DESIGNABLE true USER true)
    Q_PROPERTY(FilteringType Filtering READ filtering WRITE setFiltering DESIGNABLE true USER true)

public:
    enum class FormatType {
        Uncompressed_R8G8B8     = Texture::RGB8,
        Uncompressed_R8G8B8A8   = Texture::RGBA8,
    };

    enum class TextureType {
        Texture2D   = 1,
        Sprite,
        Cubemap
    };

    enum class FilteringType {
        None        = Texture::None,
        Bilinear    = Texture::Bilinear,
        Trilinear   = Texture::Trilinear
    };

    enum class WrapType {
        Clamp,
        Repeat,
        Mirrored
    };

    Q_ENUM(WrapType)
    Q_ENUM(FilteringType)
    Q_ENUM(TextureType)
    Q_ENUM(FormatType)

public:
    TextureImportSettings();

    TextureType textureType() const;
    void setTextureType(TextureType type);

    FormatType formatType() const;
    void setFormatType(FormatType type);

    FilteringType filtering() const;
    void setFiltering(FilteringType type);

    WrapType wrap() const;
    void setWrap(WrapType wrap);

    bool lod() const;
    void setLod(bool lod);

private:
    QString typeName() const Q_DECL_OVERRIDE;

protected:
    TextureType   m_TextureType;

    FormatType    m_FormType;

    FilteringType m_Filtering;

    WrapType      m_Wrap;

    bool          m_Lod;
};

class TextureConverter : public IConverter {
public:
    void convertTexture(TextureImportSettings *settings, Texture *texture);
    void convertSprite(TextureImportSettings *settings, Sprite *sprite);

private:
    QStringList suffixes() const Q_DECL_OVERRIDE { return {"bmp", "dds", "jpg", "jpeg", "png", "tga", "ico", "tif"}; }
    uint8_t convertFile(IConverterSettings *settings) Q_DECL_OVERRIDE;

    IConverterSettings *createSettings() const Q_DECL_OVERRIDE;

    Actor *createActor(const QString &guid) const Q_DECL_OVERRIDE;
};

#endif // TEXTURECONVERTER_H
