#include "textureconverter.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize2.h>

#include <url.h>
#include <bson.h>
#include <engine.h>
#include <components/actor.h>
#include <components/spriterender.h>
#include <components/meshrender.h>

#include <resources/resource.h>
#include <resources/material.h>
#include <resources/sprite.h>

#include <editor/projectsettings.h>

#define FORMAT_VERSION 10

TextureImportSettings::TextureImportSettings() :
        m_assetType(AssetType::Texture2D),
        m_filtering(FilteringType::None),
        m_wrap(WrapType::Repeat),
        m_pixels(100),
        m_lod(false),
        m_compressed(false) {

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

bool TextureImportSettings::compressed() const {
    return m_compressed;
}
void TextureImportSettings::setCompressed(bool compressed) {
    if(m_compressed != compressed) {
        m_compressed = compressed;
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

void TextureConverter::init() {
    AssetConverter::init();

    for(auto &it : suffixes()) {
        AssetConverterSettings::setDefaultIconPath(it, ":/Style/styles/dark/images/texture.svg");
    }
    AssetConverterSettings::setDefaultIconPath("Texture", ":/Style/styles/dark/images/texture.svg");
}

AssetConverter::ReturnCode TextureConverter::convertFile(AssetConverterSettings *settings) {
    Texture *texture = nullptr;

    settings->info().type = MetaType::name<Texture>();

    TextureImportSettings *s = dynamic_cast<TextureImportSettings *>(settings);
    if(s) {
        texture = convertTexture(s);
        if(s->assetType() == TextureImportSettings::AssetType::Sprite) {
            convertSprite(texture, s);
        }

        uint32_t uuid = settings->info().id;
        if(uuid == 0) {
            uuid = Engine::generateUUID();
            settings->info().id = uuid;
        }

        if(texture->uuid() != uuid) {
            Engine::replaceUUID(texture, uuid);
        }

        return settings->saveBinary(Engine::toVariant(texture), settings->absoluteDestination());
    }

    return InternalError;
}

Texture *TextureConverter::convertTexture(TextureImportSettings *settings) const {
    Texture *texture = Engine::loadResource<Texture>(settings->destination());
    if(texture == nullptr) {
        texture = Engine::objectCreate<Texture>(settings->destination());
    }

    int32_t width = 1;
    int32_t height = 1;
    int32_t channels;

    uint8_t *sourceData = stbi_load(settings->source().data(), &width, &height, &channels, 4);

    if(sourceData) {
        texture->clear();

        int format = Texture::RGBA8;
        channels = 4;

        texture->setFormat(format);
        texture->setFiltering(Texture::FilteringType(settings->filtering()));
        texture->setWrap(Texture::WrapType(settings->wrap()));

        std::list<ByteArray> sides;
        if(settings->assetType() == TextureImportSettings::AssetType::Cubemap) {
            std::list<Vector2> positions;
            float ratio = (float)width / (float)height;
            texture->resize(width, height);
            if(ratio == 6.0f / 1.0f) { // Row
                texture->resize(width / 6, height);
                for(int i = 0; i < 6; i++) {
                    positions.push_back(Vector2(i * texture->width(), 0));
                }
            } else if(ratio == 1.0f / 6.0f) { // Column
                texture->resize(width, height / 6);
                for(int i = 0; i < 6; i++) {
                    positions.push_back(Vector2(0, i * texture->height()));
                }
            } else if(ratio == 4.0f / 3.0f) { // Horizontal cross
                texture->resize(width / 4, height / 3);
                positions.push_back(Vector2(2 * texture->width(), 1 * texture->height()));
                positions.push_back(Vector2(0 * texture->width(), 1 * texture->height()));
                positions.push_back(Vector2(1 * texture->width(), 0 * texture->height()));
                positions.push_back(Vector2(1 * texture->width(), 2 * texture->height()));
                positions.push_back(Vector2(1 * texture->width(), 1 * texture->height()));
                positions.push_back(Vector2(3 * texture->width(), 1 * texture->height()));
            } else if(ratio == 3.0f / 4.0f) { // Vertical cross
                texture->resize(width / 3, height / 4);
                positions.push_back(Vector2(1 * texture->width(), 1 * texture->height()));
                positions.push_back(Vector2(1 * texture->width(), 3 * texture->height()));
                positions.push_back(Vector2(1 * texture->width(), 0 * texture->height()));
                positions.push_back(Vector2(1 * texture->width(), 2 * texture->height()));
                positions.push_back(Vector2(0 * texture->width(), 1 * texture->height()));
                positions.push_back(Vector2(2 * texture->width(), 1 * texture->height()));
            }

            ByteArray result;
            result.resize(width * height * channels);

            for(const Vector2 &it : positions) {
                copyRegion(sourceData, Vector2(width, height), channels, result, it, Vector2(texture->width(), texture->height()));

                sides.push_back(result);
            }
        } else if(settings->assetType() == TextureImportSettings::AssetType::Texture3D) {
            float ratio = (float)width / (float)height;

            ByteArray result;

            if(ratio > 1.0f) { // Row
                texture->resize(height, height);
                int32_t depth = width / height;
                texture->setDepth(depth);

                result.resize(height * height * depth * channels);

                for(int d = 0; d < texture->depth(); d++) {
                    copyRegion(sourceData, Vector2(width, height), channels, result,
                               Vector2(texture->width() * d, 0), Vector2(texture->width(), texture->height()));
                }
            } else { // Column
                texture->resize(width, width);
                int32_t depth = height / width;
                texture->setDepth(depth);

                result.resize(width * width * depth * channels);

                for(int d = 0; d < texture->depth(); d++) {
                    copyRegion(sourceData, Vector2(width, height), channels, result,
                               Vector2(0, texture->height() * d), Vector2(texture->width(), texture->height()));
                }
            }

            sides.push_back(result);
        } else {
            texture->resize(width, height);

            ByteArray result;
            result.resize(width * height * channels);

            copyRegion(sourceData, Vector2(width, height), channels, result, Vector2(), Vector2(width, height), true);

            sides.push_back(result);
        }

        texture->clear();

        if(settings->compressed()) {
            int method = Texture::BC7; // Desktop

            TString platform = ProjectSettings::instance()->currentPlatformName();
            if(platform.contains("webgl")) {
                method = Texture::BC3;
                if(channels == 3) {
                    method = Texture::BC1;
                }
            } else if(platform.contains("android")) {
                method = Texture::ETC2;
                if(channels == 3) {
                    method = Texture::ETC1;
                }
            } else if(platform.contains("ios")) {
                method = Texture::PVRTC;
            } else if(platform.contains("tvos")) {
                method = Texture::ASTC;
            }

            texture->setCompress(method);
        }

        for(const ByteArray &side : sides) {
            Texture::Surface surface;

            surface.push_back(side);

            // Mip map creation
            if(settings->lod()) {
                int32_t w = texture->width();
                int32_t h = texture->height();
                int32_t d = texture->depth();

                ByteArray origin = side;
                while(w > 1 && h > 1 ) {
                    int32_t originW = w;
                    int32_t originH = h;
                    int32_t originD = d;

                    w = MAX(originW / 2, 1);
                    h = MAX(originH / 2, 1);
                    d = MAX(originD / 2, 1);

                    ByteArray mip;
                    mip.resize(w * h * d * channels);

                    stbir_resize(origin.data(), originW, originH, 0,
                                 mip.data(), w, h, 0, static_cast<stbir_pixel_layout>(channels),
                                 STBIR_TYPE_UINT8, STBIR_EDGE_CLAMP, STBIR_FILTER_BOX);
                    origin = mip;
                    surface.push_back(mip);
                }
            }

            texture->addSurface(surface);
        }

        if(texture->compress() != Texture::Uncompressed) {
            compress(texture);
        }

        texture->setDirty();

        stbi_image_free(sourceData);
    }

    return texture;
}

void TextureConverter::copyRegion(const uint8_t *sourcedata, const Vector2 &sourceSize, int channels, ByteArray &data, const Vector2 &pos, const Vector2 &size, bool mirror) const {
    for(int y = 0; y < size.y; y++) {
        for(int x = 0; x < size.x; x++) {
            int srcY = pos.y + ((mirror) ? sourceSize.y - y - 1 : y);
            int srcIndex = (srcY * sourceSize.x + (pos.x + x)) * channels;
            int dstIndex = (y * size.x + x) * channels;

            for(int c = 0; c < channels; c++) {
                data[dstIndex + c] = sourcedata[srcIndex + c];
            }
        }
    }
}

void TextureConverter::convertSprite(Texture *texture, TextureImportSettings *settings) const {
    Url dst(settings->absoluteDestination());

    for(auto &it : settings->elements()) {
        ResourceSystem::ResourceInfo info = settings->subItem(it.first, true);

        Sprite *sprite = Engine::loadResource<Sprite>(info.uuid);
        if(sprite == nullptr) {
            sprite = Engine::objectCreate<Sprite>(info.uuid);
        }

        sprite->setTexture(texture);
        sprite->setPixelsPerUnit(settings->pixels());

        auto value = it.second;

        sprite->setBounds(Vector4(value.min.x, value.min.y, value.max.x, value.max.y));
        sprite->setBorder(Vector4(value.borderMin.x, value.borderMin.y, value.borderMax.x, value.borderMax.y));
        sprite->setPivot(value.pivot);
        sprite->setMode(Sprite::Sliced);

        AssetConverter::ReturnCode result = settings->saveBinary(Engine::toVariant(sprite), dst.absoluteDir() + "/" + info.uuid);
        if(result == AssetConverter::Success) {
            info.id = sprite->uuid();
            info.type = MetaType::name<Sprite>();
            settings->setSubItem(it.first, info);
        }
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
