#ifndef TEXTURECONVERTER_H
#define TEXTURECONVERTER_H

#include <resources/texture.h>
#include <resources/sprite.h>

#include <editor/assetconverter.h>

#include <QRect>

class TextureImportSettings : public AssetConverterSettings {
    Q_OBJECT

    Q_PROPERTY(AssetType Type READ assetType WRITE setAssetType DESIGNABLE true USER true)
    Q_PROPERTY(WrapType Wrap READ wrap WRITE setWrap DESIGNABLE true USER true)
    Q_PROPERTY(bool MIP_maping READ lod WRITE setLod DESIGNABLE true USER true)
    Q_PROPERTY(FilteringType Filtering READ filtering WRITE setFiltering DESIGNABLE true USER true)

public:
    enum class AssetType {
        Texture2D = 1,
        Sprite,
        Cubemap
    };

    enum class FilteringType {
        None = Texture::None,
        Bilinear = Texture::Bilinear,
        Trilinear = Texture::Trilinear
    };

    enum class WrapType {
        Clamp,
        Repeat,
        Mirrored
    };

    Q_ENUM(WrapType)
    Q_ENUM(FilteringType)
    Q_ENUM(AssetType)

    struct Element {
        Vector2 m_min;
        Vector2 m_max;

        Vector2 m_borderMin;
        Vector2 m_borderMax;

        Vector2 m_pivot = Vector2(0.5f);
    };
    typedef map<string, Element> ElementMap;

public:
    TextureImportSettings();

    AssetType assetType() const;
    void setAssetType(AssetType type);

    FilteringType filtering() const;
    void setFiltering(FilteringType type);

    WrapType wrap() const;
    void setWrap(WrapType wrap);

    bool lod() const;
    void setLod(bool lod);

    const ElementMap &elements() const;
    string setElement(const Element &element, const string &key = string());
    void removeElement(const string &key);

private:
    QJsonObject subItemData(const QString &key) const Q_DECL_OVERRIDE;
    void setSubItemData(const QString &name, const QJsonObject &data) Q_DECL_OVERRIDE;

    string findFreeElementName(const string &name);

    QStringList typeNames() const Q_DECL_OVERRIDE;
    QString typeName() const Q_DECL_OVERRIDE;

    QString defaultIcon(QString) const Q_DECL_OVERRIDE;

protected:
    AssetType m_assetType;

    FilteringType m_filtering;

    WrapType m_wrap;

    ElementMap m_elements;

    bool m_lod;

};

class TextureConverter : public AssetConverter {
public:
    void convertTexture(Texture *texture, TextureImportSettings *settings);
    void convertSprite(Sprite *sheet, TextureImportSettings *settings);

private:
    QStringList suffixes() const Q_DECL_OVERRIDE { return {"bmp", "dds", "jpg", "jpeg", "png", "tga", "ico", "tif"}; }
    ReturnCode convertFile(AssetConverterSettings *settings) Q_DECL_OVERRIDE;

    AssetConverterSettings *createSettings() const Q_DECL_OVERRIDE;

    Actor *createActor(const AssetConverterSettings *settings, const QString &guid) const Q_DECL_OVERRIDE;
};

#endif // TEXTURECONVERTER_H
