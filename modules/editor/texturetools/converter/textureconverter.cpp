#include "textureconverter.h"

#include <QImage>

#include <cstring>

#include <url.h>
#include <bson.h>
#include <engine.h>
#include <components/actor.h>
#include <components/spriterender.h>
#include <components/meshrender.h>

#include <resources/resource.h>
#include <resources/material.h>
#include <resources/sprite.h>

#define FORMAT_VERSION 9

void copyData(uint8_t *dst, const uchar *src, uint32_t size, uint8_t channels) {
    if(channels == 3) {
        uint32_t m = 0;
        for(uint32_t i = 0; i < size; i++) {
            dst[i] = src[m];

            if(i % channels == 2) {
                m++;
            }
            m++;
        }
    } else {
        memcpy(dst, src, size);
    }
}

TextureImportSettings::TextureImportSettings() :
        m_assetType(AssetType::Texture2D),
        m_filtering(FilteringType::None),
        m_wrap(WrapType::Repeat),
        m_pixels(100),
        m_lod(false) {

    setVersion(FORMAT_VERSION);
}

int TextureImportSettings::assetType() const {
    return m_assetType;
}

void TextureImportSettings::setAssetType(int type) {
    if(m_assetType != type) {
        m_assetType = type;

        setModified();
    }
}

int TextureImportSettings::filtering() const {
    return m_filtering;
}
void TextureImportSettings::setFiltering(int type) {
    if(m_filtering != type) {
        m_filtering = type;
        setModified();
    }
}

int TextureImportSettings::wrap() const {
    return m_wrap;
}
void TextureImportSettings::setWrap(int wrap) {
    if(m_wrap != wrap) {
        m_wrap = WrapType(wrap);
        setModified();
    }
}

bool TextureImportSettings::lod() const {
    return m_lod;
}
void TextureImportSettings::setLod(bool lod) {
    if(m_lod != lod) {
        m_lod = lod;
        setModified();
    }
}

int TextureImportSettings::pixels() const {
    return m_pixels;
}

void TextureImportSettings::setPixels(int pixels) {
    if(m_pixels != pixels) {
        m_pixels = pixels;
        setModified();
    }
}

TString TextureImportSettings::findFreeElementName(const TString &name) {
    TString newName = name;
    if(!newName.isEmpty()) {
        int32_t i = 0;
        while(subItem(newName + "_" + TString::number(i)).uuid.isEmpty() == false) {
            i++;
        }
        return (newName + "_" + TString::number(i));
    }
    return "Element";
}

TextureImportSettings::ElementMap &TextureImportSettings::elements() {
    return m_elements;
}

TString TextureImportSettings::setElement(const Element &element, const TString &key) {
    Url info(source());

    TString path = key;
    if(path.isEmpty()) {
        path = findFreeElementName(info.baseName());
    }

    ResourceSystem::ResourceInfo resInfo = subItem(path, true);
    m_elements[path] = element;

    resInfo.type = MetaType::name<Mesh>();

    setSubItem(path, resInfo);

    setModified();
    return path;
}

void TextureImportSettings::removeElement(const TString &key) {
    m_elements.erase(key);

    m_subItems.erase(key);

    setModified();
}

TString TextureImportSettings::propertyAllias(const TString &name) const {
    static const std::map<TString, TString> map {
        {"type", "Type"},
        {"wrap", "Wrap"},
        {"mipMaping", "MIP_Maping"},
        {"filtering", "Filtering"}
    };

    auto it = map.find(name);
    if(it != map.end()) {
        return it->second;
    }

    return name;
}

StringList TextureImportSettings::typeNames() const {
    return { "Texture", "Sprite" };
}

TString TextureImportSettings::typeName() const {
    if(assetType() == TextureImportSettings::AssetType::Sprite) {
        return typeNames().back();
    }
    return typeNames().front();
}

TString TextureImportSettings::defaultIconPath(const TString &) const {
    return ":/Style/styles/dark/images/texture.svg";
}

VariantMap TextureImportSettings::saveUserData() const {
    VariantMap result;

    for(auto &it : m_elements) {
        TextureImportSettings::Element element = it.second;

        VariantMap data;

        data["min"] = element.min;
        data["max"] = element.max;

        data["borderMin"] = element.borderMin;
        data["borderMax"] = element.borderMax;

        data["pivot"] = element.pivot;

        result[it.first] = data;
    }

    return result;
}

void TextureImportSettings::loadUserData(const VariantMap &data) {
    for(auto &it : data) {
        if(it.second.type() == MetaType::VARIANTMAP) {
            VariantMap fields = it.second.toMap();

            TextureImportSettings::Element element;

            element.min = fields["min"].toVector2();
            element.max = fields["max"].toVector2();

            element.borderMin = fields["borderMin"].toVector2();
            element.borderMax = fields["borderMax"].toVector2();

            element.pivot = fields["pivot"].toVector2();

            m_elements[it.first] = element;
        }
    }
}

void TextureImportSettings::setSubItemData(const TString &name, const Variant &data) {
    VariantMap map = data.toMap();

    VariantMap d = map["data"].toMap();

    TextureImportSettings::Element element;

    element.min.x = d["x"].toInt();
    element.min.y = d["y"].toInt();
    element.max.x = element.min.x + d["w"].toInt();
    element.max.y = element.min.y + d["h"].toInt();

    element.borderMin.x = d["l"].toInt();
    element.borderMax.x = d["r"].toInt();
    element.borderMax.y = d["t"].toInt();
    element.borderMin.y = d["b"].toInt();

    element.pivot = Vector2(d["pivotX"].toFloat(), d["pivotY"].toFloat());

    m_elements[name] = element;
}

AssetConverter::ReturnCode TextureConverter::convertFile(AssetConverterSettings *settings) {
    Resource *resource = nullptr;

    settings->info().type = settings->typeName();

    TextureImportSettings *s = dynamic_cast<TextureImportSettings *>(settings);
    if(s) {
        if(s->assetType() == TextureImportSettings::AssetType::Sprite) {
            Sprite *sprite = Engine::loadResource<Sprite>(settings->destination());
            if(sprite == nullptr) {
                sprite = Engine::objectCreate<Sprite>(settings->destination());
            }
            convertSprite(sprite, s);
            resource = sprite;
        } else {
            Texture *texture = Engine::loadResource<Texture>(settings->destination());
            if(texture == nullptr) {
                texture = Engine::objectCreate<Texture>(settings->destination());
            }
            convertTexture(texture, s);
            resource = texture;
        }

        settings->info().id = resource->uuid();

        return settings->saveBinary(Engine::toVariant(resource), settings->absoluteDestination());
    }

    return InternalError;
}

void TextureConverter::convertTexture(Texture *texture, TextureImportSettings *settings) {
    uint8_t channels = 4;
    QImage src(settings->source().data());
    QImage img = src.convertToFormat(QImage::Format_RGBA8888);

    texture->clear();

    texture->setFormat(Texture::RGBA8);
    texture->setFiltering(Texture::FilteringType(settings->filtering()));
    texture->setWrap(Texture::WrapType(settings->wrap()));

    QList<QImage> sides;
    if(settings->assetType() == TextureImportSettings::AssetType::Cubemap) {
        QList<QPoint> positions;
        float ratio = (float)img.width() / (float)img.height();
        texture->resize(img.width(), img.height());
        if(ratio == 6.0f / 1.0f) { // Row
            texture->resize(img.width() / 6, img.height());
            for(int i = 0; i < 6; i++) {
                positions.push_back(QPoint(i * texture->width(), 0));
            }
        } else if(ratio == 1.0f / 6.0f) { // Column
            texture->resize(img.width(), img.height() / 6);
            for(int i = 0; i < 6; i++) {
                positions.push_back(QPoint(0, i * texture->height()));
            }
        } else if(ratio == 4.0f / 3.0f) { // Horizontal cross
            texture->resize(img.width() / 4, img.height() / 3);
            positions.push_back(QPoint(2 * texture->width(), 1 * texture->height()));
            positions.push_back(QPoint(0 * texture->width(), 1 * texture->height()));
            positions.push_back(QPoint(1 * texture->width(), 0 * texture->height()));
            positions.push_back(QPoint(1 * texture->width(), 2 * texture->height()));
            positions.push_back(QPoint(1 * texture->width(), 1 * texture->height()));
            positions.push_back(QPoint(3 * texture->width(), 1 * texture->height()));
        } else if(ratio == 3.0f / 4.0f) { // Vertical cross
            texture->resize(img.width() / 3, img.height() / 4);
            positions.push_back(QPoint(1 * texture->width(), 1 * texture->height()));
            positions.push_back(QPoint(1 * texture->width(), 3 * texture->height()));
            positions.push_back(QPoint(1 * texture->width(), 0 * texture->height()));
            positions.push_back(QPoint(1 * texture->width(), 2 * texture->height()));
            positions.push_back(QPoint(0 * texture->width(), 1 * texture->height()));
            positions.push_back(QPoint(2 * texture->width(), 1 * texture->height()));
        }

        QRect sub;
        sub.setSize(QSize(texture->width(), texture->height()));
        foreach(const QPoint &it, positions) {
            sub.moveTo(it);
            sides.push_back(img.copy(sub));
        }
    } else if(settings->assetType() == TextureImportSettings::AssetType::Texture3D) {
        float ratio = (float)img.width() / (float)img.height();
        if(ratio > 1.0f) { // Row
            texture->resize(img.height(), img.height());
            texture->setDepth(img.width() / img.height());

            QImage result(texture->width(), texture->height() * texture->depth(), QImage::Format_RGBA8888);

            for(int d = 0; d < texture->depth(); d++) {
                for(int h = 0; h < texture->height(); h++) {
                    for(int w = 0; w < texture->width(); w++) {
                        result.setPixelColor(w, texture->height() * d + h, img.pixelColor(texture->width() * d + w, h));
                    }
                }
            }
            img = result;

        } else { // Column
            texture->resize(img.width(), img.width());
            texture->setDepth(img.height() / img.width());
        }

        sides.push_back(img);
    } else {
        texture->resize(img.width(), img.height());
        sides.push_back(img.mirrored());
    }

    texture->clear();

    int i = 0;
    foreach(const QImage &it, sides) {
        Texture::Surface surface;

        VariantList lods;

        int w = texture->width();
        int h = texture->height();
        int d = texture->depth();

        ByteArray data;
        uint32_t size = w * h * d * channels;
        if(size) {
            data.resize(size);
            copyData(data.data(), it.constBits(), size, channels);
        }
        surface.push_back(data);

        if(settings->lod()) {
            QImage mip = it;
            while(w > 1 && h > 1 ) {
                w = MAX(w / 2, 1);
                h = MAX(h / 2, 1);
                d = MAX(d / 2, 1);

                mip = mip.scaled(w, h, Qt::IgnoreAspectRatio);
                size = w * h * d * channels;
                if(size) {
                    data.resize(size);
                    copyData(&data[0], mip.constBits(), size, channels);
                }
                surface.push_back(data);
            }
        }
        texture->addSurface(surface);

        i++;
    }

    texture->setDirty();
}

uint32_t TextureConverter::toMeta(int type) {
    if(type == TextureImportSettings::AssetType::Sprite) {
        return MetaType::type<Sprite *>();
    }
    return MetaType::type<Texture *>();
}

void TextureConverter::convertSprite(Sprite *sprite, TextureImportSettings *settings) {
    TString pageName("_Page1");

    ResourceSystem::ResourceInfo info;
    Texture *texture = sprite->page();
    if(texture == nullptr) {
        info = settings->subItem(pageName, true);
        texture = Engine::loadResource<Texture>(info.uuid);
        if(texture == nullptr) {
            texture = Engine::objectCreate<Texture>(info.uuid);
        }
        sprite->addPage(texture);
    }

    convertTexture(texture, settings);

    Url dst(settings->absoluteDestination());

    AssetConverter::ReturnCode result = settings->saveBinary(Engine::toVariant(texture), dst.absoluteDir() + "/" + info.uuid);
    if(result == AssetConverter::Success) {
        info.id = texture->uuid();
        info.type = MetaType::name<Texture>();
        settings->setSubItem(pageName, info);
    }

    sprite->setPixelsPerUnit(settings->pixels());

    for(auto &it : settings->elements()) {
        auto value = it.second;

        Vector4 bounds(value.min.x, value.min.y, value.max.x, value.max.y);
        Vector4 borders(value.borderMin.x, value.borderMin.y, value.borderMax.x, value.borderMax.y);

        sprite->setRegion(Mathf::hashString(it.first), bounds, borders, value.pivot);
    }
}

AssetConverterSettings *TextureConverter::createSettings() {
    return new TextureImportSettings();
}

Actor *TextureConverter::createActor(const AssetConverterSettings *settings, const TString &guid) const {
    Resource *res = Engine::loadResource<Resource>(guid);
    Sprite *sheet = dynamic_cast<Sprite *>(res);
    if(sheet) {
        Actor *actor = Engine::composeActor<SpriteRender>("");
        SpriteRender *render = actor->getComponent<SpriteRender>();
        render->setSprite(sheet);

        return actor;
    } else {
        Texture *texture = dynamic_cast<Texture *>(res);
        if(texture) {
            if(!texture->isCubemap()) {
                Actor *actor = Engine::composeActor<SpriteRender>("");
                SpriteRender *render = actor->getComponent<SpriteRender>();
                render->setSize(Vector2(texture->width(), texture->height()));
                render->setTexture(texture);

                return actor;
            } else {
                Actor *actor = Engine::composeActor<MeshRender>("");
                MeshRender *render = actor->getComponent<MeshRender>();
                render->setMesh(Engine::loadResource<Mesh>(".embedded/sphere.fbx/Sphere001"));
                render->setMaterial(Engine::loadResource<Material>(".embedded/cubemap.shader"));
                render->materialInstance(0)->setTexture("mainTexture", texture);

                return actor;
            }
        }
    }

    return nullptr;
}
