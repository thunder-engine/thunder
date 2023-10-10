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
    Q_PROPERTY(WrapType Wrap READ wrap WRITE setWrap DESIGNABLE true USER true)
    Q_PROPERTY(bool MIP_maping READ lod WRITE setLod DESIGNABLE true USER true)
    Q_PROPERTY(FilteringType Filtering READ filtering WRITE setFiltering DESIGNABLE true USER true)

public:
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

    struct Element {
        Element() {
            m_borderL = 0;
            m_borderR = 0;
            m_borderT = 0;
            m_borderB = 0;
            m_pivot = Vector2(0.5f);
        }
        QRect m_rect;
        int m_borderL;
        int m_borderR;
        int m_borderT;
        int m_borderB;
        Vector2 m_pivot;
    };
    typedef QMap<QString, Element> ElementMap;

public:
    TextureImportSettings();

    TextureType textureType() const;
    void setTextureType(TextureType type);

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
    QJsonObject subItemData(const QString &key) const Q_DECL_OVERRIDE;
    void setSubItemData(const QString &name, const QJsonObject &data) Q_DECL_OVERRIDE;

    QString findFreeElementName(const QString &name);

    QStringList typeNames() const Q_DECL_OVERRIDE;
    QString typeName() const Q_DECL_OVERRIDE;

    QString defaultIcon(QString) const Q_DECL_OVERRIDE;

protected:
    TextureType   m_TextureType;

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

    Actor *createActor(const AssetConverterSettings *settings, const QString &guid) const Q_DECL_OVERRIDE;
};

#endif // TEXTURECONVERTER_H
