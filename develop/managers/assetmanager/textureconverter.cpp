#include "textureconverter.h"

#include <QImage>
#include <QFile>
#include <QFileInfo>

#include <cstring>

#include <bson.h>
#include <engine.h>
#include <components/actor.h>
#include <components/spriterender.h>
#include <resources/resource.h>
#include <resources/material.h>

#define FORMAT_VERSION 1

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
        m_FormType(FormatType::Uncompressed_R8G8B8),
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
        emit updated();
    }
}

TextureImportSettings::FormatType TextureImportSettings::formatType() const {
    return m_FormType;
}
void TextureImportSettings::setFormatType(FormatType type) {
    if(m_FormType != type) {
        m_FormType = type;
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

QString TextureImportSettings::typeName() const {
    if(m_TextureType == TextureType::Sprite) {
        return "Sprite";
    }
    return IConverterSettings::typeName();
}

uint8_t TextureConverter::convertFile(IConverterSettings *settings) {
    Resource *resource = nullptr;

    TextureImportSettings *s = dynamic_cast<TextureImportSettings *>(settings);
    if(s) {
        if(s->textureType() == TextureImportSettings::TextureType::Sprite) {
            Sprite *sprite = new Sprite;
            convertSprite(s, sprite);
            resource = sprite;

        } else {
            Texture *texture = new Texture;
            convertTexture(s, texture);
            resource = texture;
        }
    }

    QFile file(settings->absoluteDestination());
    if(file.open(QIODevice::WriteOnly)) {
        ByteArray data = Bson::save( Engine::toVariant(resource) );
        file.write((const char *)&data[0], data.size());
        file.close();
    }

    delete resource;

    return 0;
}

void TextureConverter::convertTexture(TextureImportSettings *settings, Texture *texture) {
    uint8_t channels;
    QImage src(settings->source());
    QImage img;
    switch(settings->formatType()) {
        case TextureImportSettings::FormatType::Uncompressed_R8G8B8: {
            img = src.convertToFormat(QImage::Format_RGB32).rgbSwapped();
            channels = 3;
        } break;
        default: {
            img = src.convertToFormat(QImage::Format_RGBA8888);
            channels = 4;
        } break;
    }

    texture->clear();

    texture->setFormat((channels == 3) ? Texture::RGB8 : Texture::RGBA8);
    texture->setFiltering(Texture::FilteringType(settings->filtering()));
    texture->setWrap(Texture::WrapType(settings->wrap()));

    QList<QImage> sides;
    if(texture->isCubemap()) {
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
            while(w > 1 || h > 1 ) {
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
}

IConverterSettings *TextureConverter::createSettings() const {
    return new TextureImportSettings();
}

Actor *TextureConverter::createActor(const QString &guid) const {
    Actor *object = Engine::objectCreate<Actor>("");

    SpriteRender *render = static_cast<SpriteRender *>(object->addComponent("SpriteRender"));
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
