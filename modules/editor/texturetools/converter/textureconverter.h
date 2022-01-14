#ifndef TEXTURECONVERTER_H
#define TEXTURECONVERTER_H

#include <resources/texture.h>
#include <resources/sprite.h>

#include <editor/assetconverter.h>

#include <QRect>

class QImage;

class TextureImportSettings : public AssetConverterSettings {
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

    struct Element {
        Element() {
            m_BorderL = 0;
            m_BorderR = 0;
            m_BorderT = 0;
            m_BorderB = 0;
            m_Pivot = Vector2(0.5f);
        }
        QRect m_Rect;
        int m_BorderL;
        int m_BorderR;
        int m_BorderT;
        int m_BorderB;
        Vector2 m_Pivot;
    };
    typedef QMap<QString, Element> ElementMap;

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

    ElementMap elements() const;
    QString setElement(const Element &element, const QString &key = QString());
    void removeElement(const QString &key);

private:
    QJsonObject subItemData(const QString &key) const override;
    void setSubItemData(const QString &name, const QJsonObject &data) override;

    QString findFreeElementName(const QString &name);

protected:
    TextureType   m_TextureType;

    FormatType    m_FormType;

    FilteringType m_Filtering;

    WrapType      m_Wrap;

    ElementMap    m_Elements;

    bool          m_Lod;
};

class TextureConverter : public AssetConverter {
public:
    void convertTexture(TextureImportSettings *settings, Texture *texture);
    void convertSprite(TextureImportSettings *settings, Sprite *sprite);

private:
    QStringList suffixes() const Q_DECL_OVERRIDE { return {"bmp", "dds", "jpg", "jpeg", "png", "tga", "ico", "tif"}; }
    ReturnCode convertFile(AssetConverterSettings *settings) Q_DECL_OVERRIDE;

    AssetConverterSettings *createSettings() const Q_DECL_OVERRIDE;

    Actor *createActor(const QString &guid) const Q_DECL_OVERRIDE;
};

#endif // TEXTURECONVERTER_H
