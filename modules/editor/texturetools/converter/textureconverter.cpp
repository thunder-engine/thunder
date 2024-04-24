#include "textureconverter.h"

#include <QImage>
#include <QFile>
#include <QFileInfo>
#include <QJsonObject>
#include <QUuid>

#include <cstring>

#include <bson.h>
#include <engine.h>
#include <components/actor.h>
#include <components/spriterender.h>
#include <resources/resource.h>
#include <resources/material.h>

#define FORMAT_VERSION 8

static hash<string> hash_str;

void copyData(int8_t *dst, const uchar *src, uint32_t size, uint8_t channels) {
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
        m_lod(false) {

    setVersion(FORMAT_VERSION);
    setType(MetaType::type<Texture *>());
}

TextureImportSettings::AssetType TextureImportSettings::assetType() const {
    return m_assetType;
}
void TextureImportSettings::setAssetType(AssetType type) {
    if(m_assetType != type) {
        m_assetType = type;
        if(m_assetType == AssetType::Sprite) {
            setType(MetaType::type<Sprite *>());
        } else {
            setType(MetaType::type<Texture *>());
        }
        emit updated();
    }
}

TextureImportSettings::FilteringType TextureImportSettings::filtering() const {
    return m_filtering;
}
void TextureImportSettings::setFiltering(FilteringType type) {
    if(m_filtering != type) {
        m_filtering = type;
        emit updated();
    }
}

TextureImportSettings::WrapType TextureImportSettings::wrap() const {
    return m_wrap;
}
void TextureImportSettings::setWrap(WrapType wrap) {
    if(m_wrap != wrap) {
        m_wrap = WrapType(wrap);
        emit updated();
    }
}

bool TextureImportSettings::lod() const {
    return m_lod;
}
void TextureImportSettings::setLod(bool lod) {
    if(m_lod != lod) {
        m_lod = lod;
        emit updated();
    }
}

string TextureImportSettings::findFreeElementName(const string &name) {
    QString newName = name.c_str();
    if(!newName.isEmpty()) {
        int32_t i = 0;
        while(subItem(newName + QString("_%1").arg(i)).isEmpty() == false) {
            i++;
        }
        return (newName + QString("_%1").arg(i)).toStdString();
    }
    return "Element";
}

const TextureImportSettings::ElementMap &TextureImportSettings::elements() const {
    return m_elements;
}

string TextureImportSettings::setElement(const Element &element, const string &key) {
    QFileInfo info(source());

    string path = key;
    if(path.empty()) {
        path = findFreeElementName(info.baseName().toStdString());
    }

    QString uuid = subItem(path.c_str());
    if(uuid.isEmpty()) {
        uuid = QUuid::createUuid().toString();
    }
    m_elements[path] = element;
    setSubItem(path.c_str(), uuid, MetaType::type<Mesh *>());

    emit updated();
    return path;
}

void TextureImportSettings::removeElement(const string &key) {
    m_elements.erase(key);

    m_subItems.remove(key.c_str());
    m_subTypes.remove(key.c_str());

    emit updated();
}

QStringList TextureImportSettings::typeNames() const {
    return { "Texture", "Sprite" };
}

QString TextureImportSettings::typeName() const {
    if(assetType() == TextureImportSettings::AssetType::Sprite) {
        return typeNames().constLast();
    }
    return typeNames().constFirst();
}

QString TextureImportSettings::defaultIcon(QString) const {
    return ":/Style/styles/dark/images/texture.svg";
}
#include <QDebug>
QJsonObject TextureImportSettings::subItemData(const QString &key) const {
    QJsonObject result;

    auto it = m_elements.find(key.toStdString());
    if(it != m_elements.end()) {
        TextureImportSettings::Element element = it->second;

        result["type"] = 0;

        QJsonObject data;

        data["x"] = (int)element.m_min.x;
        data["y"] = (int)element.m_min.y;
        data["w"] = int(element.m_max.x - element.m_min.x);
        data["h"] = int(element.m_max.y - element.m_min.y);

        data["l"] = (int)element.m_borderMin.x;
        data["r"] = (int)element.m_borderMax.x;
        data["t"] = (int)element.m_borderMax.y;
        data["b"] = (int)element.m_borderMin.y;

        data["pivotX"] = element.m_pivot.x;
        data["pivotY"] = element.m_pivot.y;

        result["data"] = data;
    }

    return result;
}

void TextureImportSettings::setSubItemData(const QString &name, const QJsonObject &data) {
    QJsonObject d = data.value("data").toObject();

    TextureImportSettings::Element element;

    element.m_min.x = d.value("x").toInt();
    element.m_min.y = d.value("y").toInt();
    element.m_max.x = element.m_min.x + d.value("w").toInt();
    element.m_max.y = element.m_min.y + d.value("h").toInt();

    element.m_borderMin.x = d.value("l").toInt();
    element.m_borderMax.x = d.value("r").toInt();
    element.m_borderMax.y = d.value("t").toInt();
    element.m_borderMin.y = d.value("b").toInt();

    element.m_pivot = Vector2(d.value("pivotX").toDouble(), d.value("pivotY").toDouble());

    m_elements[name.toStdString()] = element;
}

AssetConverter::ReturnCode TextureConverter::convertFile(AssetConverterSettings *settings) {
    Resource *resource = nullptr;

    TextureImportSettings *s = dynamic_cast<TextureImportSettings *>(settings);
    if(s) {
        if(s->assetType() == TextureImportSettings::AssetType::Sprite) {
            Sprite *sprite = Engine::objectCreate<Sprite>();
            convertSprite(sprite, s);
            resource = sprite;
        } else {
            Texture *texture = Engine::objectCreate<Texture>();
            convertTexture(texture, s);
            resource = texture;
        }

        QFile file(settings->absoluteDestination());
        if(file.open(QIODevice::WriteOnly)) {
            ByteArray data = Bson::save( Engine::toVariant(resource) );
            file.write((const char *)&data[0], data.size());
            file.close();
        }

        Engine::unloadResource(resource);
    }

    return Success;
}

void TextureConverter::convertTexture(Texture *texture, TextureImportSettings *settings) {
    uint8_t channels = 4;
    QImage src(settings->source());
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
    } else {
        texture->resize(img.width(), img.height());
        sides.push_back(img.mirrored());
    }

    texture->clear();

    int i = 0;
    foreach(const QImage &it, sides) {
        Texture::Surface surface;

        VariantList lods;

        ByteArray data;
        uint32_t size = it.width() * it.height() * channels;
        if(size) {
            data.resize(size);
            copyData(&data[0], it.constBits(), size, channels);
        }
        surface.push_back(data);

        if(settings->lod()) {
            /// \todo Specular convolution for cubemaps
            int w = texture->width();
            int h = texture->height();
            QImage mip = it;
            while(w > 1 && h > 1 ) {
                w = MAX(w / 2, 1);
                h = MAX(h / 2, 1);

                mip = mip.scaled(w, h, Qt::IgnoreAspectRatio);
                size = w * h * channels;
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

void TextureConverter::convertSprite(Sprite *sprite, TextureImportSettings *settings) {
    Texture *texture = Engine::objectCreate<Texture>("_Page1");
    convertTexture(texture, settings);

    sprite->addPage(texture);

    QString uuid = settings->saveSubData(Bson::save(ObjectSystem::toVariant(texture)), texture->name().c_str(), MetaType::type<Texture *>());
    Engine::setResource(texture, uuid.toStdString());

    float width = texture->width();
    float height = texture->height();

    uint32_t unitsPerPixel = 100;

    int i = 0;
    for(auto &it : settings->elements()) {
        Mesh *mesh = Engine::objectCreate<Mesh>(it.first);
        if(mesh) {
            auto value = it.second;

            Vector2 p = value.m_pivot;

            float w = (float)(value.m_max.x - value.m_min.x)  / unitsPerPixel;
            float h = (float)(value.m_max.y - value.m_min.y) / unitsPerPixel;

            float l = (float)value.m_borderMin.x / unitsPerPixel;
            float r = (float)value.m_borderMax.x / unitsPerPixel;
            float t = (float)value.m_borderMax.y / unitsPerPixel;
            float b = (float)value.m_borderMin.y / unitsPerPixel;

            mesh->setIndices({0, 1, 5, 0, 5, 4, 1, 2, 6, 1, 6, 5, 2, 3, 7, 2, 7, 6,
                              4, 5, 9, 4, 9, 8, 5, 6,10, 5,10, 9, 6, 7,11, 6,11,10,
                              8, 9,13, 8,13,12, 9,10,14, 9,14,13,10,11,15,10,15,14});

            {
                float x0 = -w * p.x;
                float x1 = -w * p.x + l;
                float x2 =  w * (1.0f - p.x) - r;
                float x3 =  w * (1.0f - p.x);

                float y0 = -h * p.y;
                float y1 = -h * p.y + b;
                float y2 =  h * (1.0f - p.y) - t;
                float y3 =  h * (1.0f - p.y);

                mesh->setVertices({
                    Vector3(x0, y0, 0.0f), Vector3(x1, y0, 0.0f), Vector3(x2, y0, 0.0f), Vector3(x3, y0, 0.0f),
                    Vector3(x0, y1, 0.0f), Vector3(x1, y1, 0.0f), Vector3(x2, y1, 0.0f), Vector3(x3, y1, 0.0f),

                    Vector3(x0, y2, 0.0f), Vector3(x1, y2, 0.0f), Vector3(x2, y2, 0.0f), Vector3(x3, y2, 0.0f),
                    Vector3(x0, y3, 0.0f), Vector3(x1, y3, 0.0f), Vector3(x2, y3, 0.0f), Vector3(x3, y3, 0.0f),
                });
            }
            {
                float x0 = (float)value.m_min.x / width;
                float x1 = (float)(value.m_min.x + value.m_borderMin.x) / width;
                float x2 = (float)(value.m_max.x - value.m_borderMax.x) / width;
                float x3 = (float)value.m_max.x / width;

                float y0 = (float)value.m_min.y / height;
                float y1 = (float)(value.m_min.y + value.m_borderMin.y) / height;
                float y2 = (float)(value.m_max.y - value.m_borderMax.y) / height;
                float y3 = (float)value.m_max.y / height;

                mesh->setUv0({
                    Vector2(x0, y0), Vector2(x1, y0), Vector2(x2, y0), Vector2(x3, y0),
                    Vector2(x0, y1), Vector2(x1, y1), Vector2(x2, y1), Vector2(x3, y1),

                    Vector2(x0, y2), Vector2(x1, y2), Vector2(x2, y2), Vector2(x3, y2),
                    Vector2(x0, y3), Vector2(x1, y3), Vector2(x2, y3), Vector2(x3, y3),
                });
            }
            {
                 mesh->setColors({
                    Vector4(1.0f), Vector4(1.0f), Vector4(1.0f), Vector4(1.0f),
                    Vector4(1.0f), Vector4(1.0f), Vector4(1.0f), Vector4(1.0f),

                    Vector4(1.0f), Vector4(1.0f), Vector4(1.0f), Vector4(1.0f),
                    Vector4(1.0f), Vector4(1.0f), Vector4(1.0f), Vector4(1.0f),
                });
            }

            QString uuid = settings->saveSubData(Bson::save(ObjectSystem::toVariant(mesh)), mesh->name().c_str(), MetaType::type<Mesh *>());
            Engine::setResource(mesh, uuid.toStdString());

            sprite->setShape(hash_str(it.first), mesh);
            i++;
        }
    }
}

AssetConverterSettings *TextureConverter::createSettings() const {
    return new TextureImportSettings();
}

Actor *TextureConverter::createActor(const AssetConverterSettings *settings, const QString &guid) const {
    Actor *object = Engine::composeActor("SpriteRender", "");

    SpriteRender *render = static_cast<SpriteRender *>(object->component("SpriteRender"));
    if(render) {
        render->setMaterial(Engine::loadResource<Material>(".embedded/DefaultSprite.mtl"));
        Resource *res = Engine::loadResource<Resource>(guid.toStdString());
        Sprite *sheet = dynamic_cast<Sprite *>(res);
        if(sheet) {
            render->setSprite(sheet);
        } else {
            Texture *texture = dynamic_cast<Texture *>(res);
            if(texture) {
                render->setTexture(texture);
            }
        }
    }

    return object;
}
