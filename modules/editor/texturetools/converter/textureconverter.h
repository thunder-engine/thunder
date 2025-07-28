#ifndef TEXTURECONVERTER_H
#define TEXTURECONVERTER_H

#include <resources/texture.h>
#include <resources/sprite.h>

#include <editor/assetconverter.h>

class TextureImportSettings : public AssetConverterSettings {
    A_OBJECT(TextureImportSettings, AssetConverterSettings, Editor)

    A_PROPERTIES(
        A_PROPERTYEX(AssetType, Type, TextureImportSettings::assetType, TextureImportSettings::setAssetType, "enum=AssetType"),
        A_PROPERTYEX(WrapType, Wrap, TextureImportSettings::wrap, TextureImportSettings::setWrap, "enum=WrapType"),
        A_PROPERTY(bool, MIP_maping, TextureImportSettings::lod, TextureImportSettings::setLod),
        A_PROPERTYEX(FilteringType, Filtering, TextureImportSettings::filtering, TextureImportSettings::setFiltering, "enum=AssetType")
    )
    A_ENUMS(
        A_ENUM(AssetType,
               A_VALUE(Texture2D),
               A_VALUE(Sprite),
               A_VALUE(Cubemap),
               A_VALUE(Texture3D)),
        A_ENUM(FilteringType,
               A_VALUE(None),
               A_VALUE(Bilinear),
               A_VALUE(Trilinear)),
        A_ENUM(WrapType,
               A_VALUE(Clamp),
               A_VALUE(Repeat),
               A_VALUE(Mirrored))
    )

public:
    enum AssetType {
        Texture2D = 1,
        Sprite,
        Cubemap,
        Texture3D
    };

    enum FilteringType {
        None = Texture::None,
        Bilinear = Texture::Bilinear,
        Trilinear = Texture::Trilinear
    };

    enum WrapType {
        Clamp,
        Repeat,
        Mirrored
    };

    struct Element {
        Vector2 m_min;
        Vector2 m_max;

        Vector2 m_saveMin;
        Vector2 m_saveMax;

        Vector2 m_borderMin;
        Vector2 m_borderMax;

        Vector2 m_saveBorderMin;
        Vector2 m_saveBorderMax;

        Vector2 m_pivot = Vector2(0.5f);

        Vector2 m_savePivot;
    };
    typedef std::map<TString, Element> ElementMap;

public:
    TextureImportSettings();

    int assetType() const;
    void setAssetType(int type);

    int filtering() const;
    void setFiltering(int type);

    int wrap() const;
    void setWrap(int wrap);

    bool lod() const;
    void setLod(bool lod);

    uint32_t pixels() const;
    void setPixels(uint32_t pixels);

    ElementMap &elements();
    TString setElement(const Element &element, const TString &key = TString());
    void removeElement(const TString &key);

private:
    Variant subItemData(const TString &key) const override;
    void setSubItemData(const TString &name, const Variant &data) override;

    TString findFreeElementName(const TString &name);

    StringList typeNames() const override;
    TString typeName() const override;

    TString defaultIconPath(const TString &) const override;

protected:
    int m_assetType;

    int m_filtering;

    int m_wrap;

    ElementMap m_elements;

    uint32_t m_pixels;

    bool m_lod;

};

class TextureConverter : public AssetConverter {
public:
    void convertTexture(Texture *texture, TextureImportSettings *settings);
    void convertSprite(Sprite *sheet, TextureImportSettings *settings);

    static uint32_t toMeta(int type);

private:
    StringList suffixes() const override { return {"bmp", "dds", "jpg", "jpeg", "png", "tga", "ico", "tif"}; }
    ReturnCode convertFile(AssetConverterSettings *settings) override;

    AssetConverterSettings *createSettings() override;

    Actor *createActor(const AssetConverterSettings *settings, const TString &guid) const override;
};

#endif // TEXTURECONVERTER_H
