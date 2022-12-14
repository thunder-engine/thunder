#include "textureconverter.h"

#include <QImage>
#include <QFile>
#include <QFileInfo>
#include <QJsonObject>
#include <QUuid>
#include <QDebug>

#include <cstring>

#include <bson.h>
#include <engine.h>
#include <components/actor.h>
#include <components/spriterender.h>
#include <resources/resource.h>
#include <resources/material.h>

#define FORMAT_VERSION 6

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
        m_TextureType(TextureType::Texture2D),
        m_Filtering(FilteringType::None),
        m_Wrap(WrapType::Repeat),
        m_Lod(false) {

    setVersion(FORMAT_VERSION);
    setType(MetaType::type<Texture *>());
}

TextureImportSettings::TextureType TextureImportSettings::textureType() const {
    return m_TextureType;
}
void TextureImportSettings::setTextureType(TextureType type) {
    if(m_TextureType != type) {
        m_TextureType = type;
        if(m_TextureType == TextureType::Sprite) {
            setType(MetaType::type<Sprite *>());
        } else {
            setType(MetaType::type<Texture *>());
        }
        emit updated();
    }
}

TextureImportSettings::FilteringType TextureImportSettings::filtering() const {
    return m_Filtering;
}
void TextureImportSettings::setFiltering(FilteringType type) {
    if(m_Filtering != type) {
        m_Filtering = type;
        emit updated();
    }
}

TextureImportSettings::WrapType TextureImportSettings::wrap() const {
    return m_Wrap;
}
void TextureImportSettings::setWrap(WrapType wrap) {
    if(m_Wrap != wrap) {
        m_Wrap = WrapType(wrap);
        emit updated();
    }
}

bool TextureImportSettings::lod() const {
    return m_Lod;
}
void TextureImportSettings::setLod(bool lod) {
    if(m_Lod != lod) {
        m_Lod = lod;
        emit updated();
    }
}

QString TextureImportSettings::findFreeElementName(const QString &name) {
    QString newName  = name;
    if(!newName.isEmpty()) {
        int32_t i = 0;
        while(subItem(newName + QString("_%1").arg(i)).isEmpty() == false) {
            i++;
        }
        return (newName + QString("_%1").arg(i));
    }
    return "Element";
}

TextureImportSettings::ElementMap TextureImportSettings::elements() const {
    return m_Elements;
}

QString TextureImportSettings::setElement(const Element &element, const QString &key) {
    QFileInfo info(source());

    QString path = key;
    if(path.isEmpty()) {
        path = findFreeElementName(info.baseName());
    }

    QString uuid = subItem(path);
    if(uuid.isEmpty()) {
        uuid = QUuid::createUuid().toString();
    }
    m_Elements[path] = element;
    setSubItem(path, uuid, MetaType::type<Mesh *>());

    emit updated();
    return path;
}

void TextureImportSettings::removeElement(const QString &key) {
    m_Elements.remove(key);

    m_subItems.remove(key);
    m_subTypes.remove(key);

    emit updated();
}

QStringList TextureImportSettings::typeNames() const {
    return { "Texture", "Sprite" };
}

QString TextureImportSettings::typeName() const {
    if(textureType() == TextureImportSettings::TextureType::Sprite) {
        return typeNames().constLast();
    }
    return typeNames().constFirst();
}

QString TextureImportSettings::defaultIcon(QString) const {
    return ":/Style/styles/dark/images/texture.svg";
}

QJsonObject TextureImportSettings::subItemData(const QString &key) const {
    QRect rect = m_Elements.value(key).m_Rect;
    QJsonObject result;
    result["type"] = 0;

    QJsonObject r;

    r["x"] = rect.x();
    r["y"] = rect.y();
    r["w"] = rect.width();
    r["h"] = rect.height();

    r["l"] = m_Elements.value(key).m_BorderL;
    r["r"] = m_Elements.value(key).m_BorderR;
    r["t"] = m_Elements.value(key).m_BorderT;
    r["b"] = m_Elements.value(key).m_BorderB;

    Vector2 pivot = m_Elements.value(key).m_Pivot;
    r["pivotX"] = pivot.x;
    r["pivotY"] = pivot.y;

    result["data"] = r;

    return result;
}

void TextureImportSettings::setSubItemData(const QString &name, const QJsonObject &data) {
    QJsonObject d = data.value("data").toObject();

    QRect rect;
    rect.setX       (d.value("x").toInt());
    rect.setY       (d.value("y").toInt());
    rect.setWidth   (d.value("w").toInt());
    rect.setHeight  (d.value("h").toInt());

    m_Elements[name].m_BorderL = d.value("l").toInt();
    m_Elements[name].m_BorderR = d.value("r").toInt();
    m_Elements[name].m_BorderT = d.value("t").toInt();
    m_Elements[name].m_BorderB = d.value("b").toInt();

    Vector2 pivot(d.value("pivotX").toDouble(), d.value("pivotY").toDouble());

    m_Elements[name].m_Rect = rect;
    m_Elements[name].m_Pivot = pivot;
}

AssetConverter::ReturnCode TextureConverter::convertFile(AssetConverterSettings *settings) {
    Resource *resource = nullptr;

    TextureImportSettings *s = dynamic_cast<TextureImportSettings *>(settings);
    if(s) {
        if(s->textureType() == TextureImportSettings::TextureType::Sprite) {
            Sprite *sprite = Engine::objectCreate<Sprite>();
            convertSprite(s, sprite);
            resource = sprite;
        } else {
            Texture *texture = Engine::objectCreate<Texture>();
            convertTexture(s, texture);
            resource = texture;
        }

        QFile file(settings->absoluteDestination());
        if(file.open(QIODevice::WriteOnly)) {
            ByteArray data = Bson::save( Engine::toVariant(resource) );
            file.write((const char *)&data[0], data.size());
            file.close();
        }

        Engine::unloadResource(resource);

        settings->setCurrentVersion(settings->version());
    }

    return Success;
}

void TextureConverter::convertTexture(TextureImportSettings *settings, Texture *texture) {
    uint8_t channels = 4;
    QImage src(settings->source());
    QImage img = src.convertToFormat(QImage::Format_RGBA8888);

    texture->clear();

    texture->setFormat(Texture::RGBA8);
    texture->setFiltering(Texture::FilteringType(settings->filtering()));
    texture->setWrap(Texture::WrapType(settings->wrap()));

    QList<QImage> sides;
    if(settings->textureType() == TextureImportSettings::TextureType::Cubemap) {
        QList<QPoint> positions;
        float ratio = (float)img.width() / (float)img.height();
        texture->setWidth(img.width());
        texture->setHeight(img.height());
        if(ratio == 6.0f / 1.0f) { // Row
            texture->setWidth(img.width() / 6);
            texture->setHeight(img.height());
            for(int i = 0; i < 6; i++) {
                positions.push_back(QPoint(i * texture->width(), 0));
            }
        } else if(ratio == 1.0f / 6.0f) { // Column
            texture->setWidth(img.width());
            texture->setHeight(img.height() / 6);
            for(int i = 0; i < 6; i++) {
                positions.push_back(QPoint(0, i * texture->height()));
            }
        } else if(ratio == 4.0f / 3.0f) { // Horizontal cross
            texture->setWidth(img.width() / 4);
            texture->setHeight(img.height() / 3);
            positions.push_back(QPoint(2 * texture->width(), 1 * texture->height()));
            positions.push_back(QPoint(0 * texture->width(), 1 * texture->height()));
            positions.push_back(QPoint(1 * texture->width(), 0 * texture->height()));
            positions.push_back(QPoint(1 * texture->width(), 2 * texture->height()));
            positions.push_back(QPoint(1 * texture->width(), 1 * texture->height()));
            positions.push_back(QPoint(3 * texture->width(), 1 * texture->height()));
        } else if(ratio == 3.0f / 4.0f) { // Vertical cross
            texture->setWidth(img.width() / 3);
            texture->setHeight(img.height() / 4);
            positions.push_back(QPoint(1 * texture->width(), 1 * texture->height()));
            positions.push_back(QPoint(1 * texture->width(), 3 * texture->height()));
            positions.push_back(QPoint(1 * texture->width(), 0 * texture->height()));
            positions.push_back(QPoint(1 * texture->width(), 2 * texture->height()));
            positions.push_back(QPoint(0 * texture->width(), 1 * texture->height()));
            positions.push_back(QPoint(2 * texture->width(), 1 * texture->height()));
        } else {
            //qDebug() << "Unsupported ratio";
        }

        QRect sub;
        sub.setSize(QSize(texture->width(), texture->height()));
        foreach(const QPoint &it, positions) {
            sub.moveTo(it);
            sides.push_back(img.copy(sub));
        }
    } else {
        texture->setWidth(img.width());
        texture->setHeight(img.height());
        sides.push_back(img.mirrored());
    }

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

void TextureConverter::convertSprite(TextureImportSettings *settings, Sprite *sprite) {
    convertTexture(settings, sprite->texture());

    float width = sprite->texture()->width();
    float height = sprite->texture()->height();

    uint32_t unitsPerPixel = 100;

    int i = 0;
    for(auto &it : settings->elements().keys()) {
        Mesh *mesh = Engine::objectCreate<Mesh>((settings->destination() + "/" + it).toStdString(), sprite);
        if(mesh) {
            auto value = settings->elements().value(it);

            QRect rect = value.m_Rect;
            Vector2 p = value.m_Pivot;

            float w = (float)rect.width()  / unitsPerPixel;
            float h = (float)rect.height() / unitsPerPixel;

            float l = (float)value.m_BorderL / unitsPerPixel;
            float r = (float)value.m_BorderR / unitsPerPixel;
            float t = (float)value.m_BorderT / unitsPerPixel;
            float b = (float)value.m_BorderB / unitsPerPixel;

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
                float x0 = (float)rect.left() / width;
                float x1 = (float)(rect.left() + value.m_BorderL) / width;
                float x2 = (float)(rect.right() - value.m_BorderR) / width;
                float x3 = (float)rect.right() / width;

                float y0 = (float)rect.top() / height;
                float y1 = (float)(rect.top() + value.m_BorderB) / height;
                float y2 = (float)(rect.bottom() - value.m_BorderT) / height;
                float y3 = (float)rect.bottom() / height;

                mesh->setUv0({
                    Vector2(x0, y0), Vector2(x1, y0), Vector2(x2, y0), Vector2(x3, y0),
                    Vector2(x0, y1), Vector2(x1, y1), Vector2(x2, y1), Vector2(x3, y1),

                    Vector2(x0, y2), Vector2(x1, y2), Vector2(x2, y2), Vector2(x3, y2),
                    Vector2(x0, y3), Vector2(x1, y3), Vector2(x2, y3), Vector2(x3, y3),
                });
            }

            sprite->setMesh(hash_str(it.toStdString()), mesh);
            i++;
        }
    }
}

AssetConverterSettings *TextureConverter::createSettings() const {
    return new TextureImportSettings();
}

Actor *TextureConverter::createActor(const QString &guid) const {
    Actor *object = Engine::composeActor("SpriteRender", "");

    SpriteRender *render = static_cast<SpriteRender *>(object->component("SpriteRender"));
    if(render) {
        render->setMaterial(Engine::loadResource<Material>(".embedded/DefaultSprite.mtl"));
        Resource *res = Engine::loadResource<Resource>(guid.toStdString());
        Sprite *sprite = dynamic_cast<Sprite *>(res);
        if(sprite) {
            render->setSprite(sprite);
        } else {
            Texture *texture = dynamic_cast<Texture *>(res);
            if(texture) {
                render->setTexture(texture);
            }
        }
    }

    return object;
}
