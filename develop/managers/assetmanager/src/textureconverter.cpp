#include "textureconverter.h"

#include <QImage>
#include <QFile>
#include <QFileInfo>

#if __linux__
#include <cstring>
#endif

#include <bson.h>
#include <resources/texture.h>

#include "textureimportsettings.h"
#include "projectmanager.h"

void copyData(const uchar *src, int8_t *dst, uint32_t size, uint8_t channels) {
    if(channels == 3) {
        uint32_t m  = 0;
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

void TextureSerial::loadUserData(const VariantMap &data) {
    Texture::loadUserData(data);

    auto it = data.find("Data");
    if(it != data.end()) {
        m_Surfaces  = (*it).second.value<VariantList>();
    }
}

VariantMap TextureSerial::saveUserData() const {
    VariantMap result;

    VariantList header;

    header.push_back((int)m_Width);
    header.push_back((int)m_Height);
    header.push_back(0); // Reserved

    header.push_back(m_Type);
    header.push_back(0); // Reserved
    header.push_back((int)m_Format);
    header.push_back((int)m_Filtering);
    header.push_back((int)m_Wrap);

    result["Header"]    = header;
    result["Data"]      = m_Surfaces;

    return result;
}

uint8_t TextureConverter::convertFile(IConverterSettings *settings) {
    TextureSerial texture;
    VariantMap data    = convertResource(settings);
    texture.loadUserData(data);

    QFile file(ProjectManager::instance()->importPath() + "/" + settings->destination());
    if(file.open(QIODevice::WriteOnly)) {
        ByteArray data  = Bson::save( Engine::toVariant(&texture) );
        file.write((const char *)&data[0], data.size());
        file.close();
    }
    return 0;
}

VariantMap TextureConverter::convertResource(IConverterSettings *settings) {
    TextureSerial texture;

    TextureImportSettings *s    = dynamic_cast<TextureImportSettings *>(settings);
    uint8_t channels;
    if(s) {
        QImage src(s->source());
        QImage img;
        switch(s->formatType()) {
            case TextureImportSettings::Uncompressed_R8G8B8: {
                img = src.convertToFormat(QImage::Format_RGB32).rgbSwapped();
                channels    = 3;
            } break;
            default: {
                img = src.convertToFormat(QImage::Format_RGBA8888);
                channels    = 4;
            } break;
        }

        texture.m_Format    = (channels == 3) ? Texture::RGB8 : Texture::RGBA8;
        texture.m_Type      = Texture::TextureType(s->textureType());
        texture.m_Filtering = Texture::FilteringType(s->filtering());
        texture.m_Wrap      = Texture::WrapType(s->wrap());

        if(!texture.m_Width || !texture.m_Height) {
            //tgaReader(settings, img);
            //return 1;
        }

        QList<QImage> sides;
        if(texture.isCubemap()) {
            QList<QPoint> positions;
            float ratio = (float)img.width() / (float)img.height();
            if(ratio == 6.0f / 1.0f) { // Row
                texture.m_Width     = img.width() / 6;
                texture.m_Height    = img.height();
                for(int i = 0; i < 6; i++) {
                    positions.push_back(QPoint(i * texture.m_Width, 0));
                }
            } else if(ratio == 1.0f / 6.0f) { // Column
                texture.m_Width     = img.width();
                texture.m_Height    = img.height() / 6;
                for(int i = 0; i < 6; i++) {
                    positions.push_back(QPoint(0, i * texture.m_Height));
                }
            } else if(ratio == 4.0f / 3.0f) { // Horizontal cross
                texture.m_Width     = img.width() / 4;
                texture.m_Height    = img.height() / 3;
                positions.push_back(QPoint(2 * texture.m_Width, 1 * texture.m_Height));
                positions.push_back(QPoint(0 * texture.m_Width, 1 * texture.m_Height));
                positions.push_back(QPoint(1 * texture.m_Width, 0 * texture.m_Height));
                positions.push_back(QPoint(1 * texture.m_Width, 2 * texture.m_Height));
                positions.push_back(QPoint(1 * texture.m_Width, 1 * texture.m_Height));
                positions.push_back(QPoint(3 * texture.m_Width, 1 * texture.m_Height));
            } else if(ratio == 3.0f / 4.0f) { // Vertical cross
                texture.m_Width     = img.width() / 3;
                texture.m_Height    = img.height() / 4;
                positions.push_back(QPoint(1 * texture.m_Width, 1 * texture.m_Height));
                positions.push_back(QPoint(1 * texture.m_Width, 3 * texture.m_Height));
                positions.push_back(QPoint(1 * texture.m_Width, 0 * texture.m_Height));
                positions.push_back(QPoint(1 * texture.m_Width, 2 * texture.m_Height));
                positions.push_back(QPoint(0 * texture.m_Width, 1 * texture.m_Height));
                positions.push_back(QPoint(2 * texture.m_Width, 1 * texture.m_Height));
            } else {
                //qDebug() << "Unsupported ratio";
                //return 1;
            }

            QRect sub;
            sub.setSize(QSize(texture.m_Width, texture.m_Height));
            foreach(const QPoint &it, positions) {
                sub.moveTo(it);
                sides.push_back(img.copy(sub));
            }

        } else {
            texture.m_Width     = img.width();
            texture.m_Height    = img.height();
            sides.push_back(img.mirrored());
        }

        foreach(const QImage &it, sides) {
            VariantList lods;

            ByteArray data;
            uint32_t size   = it.width() * it.height() * channels;
            if(size) {
                data.resize(size);
                copyData(it.constBits(), &data[0], size, channels);
            }
            lods.push_back(data);

            if(s->lod()) {
                /// \todo Specular convolution for cubemaps
                int w   = texture.m_Width;
                int h   = texture.m_Height;
                QImage mip  = it;
                while(w > 1 || h > 1 ) {
                    w   = MAX(w / 2, 1);
                    h   = MAX(h / 2, 1);

                    mip     = mip.scaled(w, h, Qt::IgnoreAspectRatio);
                    size    = w * h * channels;
                    if(size) {
                        data.resize(size);
                        copyData(mip.constBits(), &data[0], size, channels);
                    }
                    lods.push_back(data);
                }
            }
            texture.m_Surfaces.push_back(lods);
        }
    }
    return texture.saveUserData();
}

#define TGA_TYPE_COLOR 2
#define TGA_TYPE_GRAY 3

bool TextureConverter::tgaReader(IConverterSettings &settings, QImage &t) {
    unsigned char GrayMask[12]	= {0, 0, TGA_TYPE_GRAY, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    unsigned char RGBMask[12]	= {0, 0, TGA_TYPE_COLOR, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    unsigned char Header[18];

    QFile file(settings.source());
    if(file.open(QIODevice::ReadOnly)) {
        file.read((char *)Header, sizeof(Header));

        uint32_t bpp    = Header[16];

        QImage::Format format;

        switch(Header[2]) {
            case TGA_TYPE_GRAY: {
                format  = QImage::Format_Grayscale8;
            } break;
            case TGA_TYPE_COLOR: {
                if (bpp == 32) {
                    format  = QImage::Format_RGBA8888;
                } else {
                    format  = QImage::Format_RGB888;
                }
            } break;
            default: break;
        }

        bpp >>=	3;
        uint32_t Width  = (Header[13]<< 8) | Header[12];
        uint32_t Height = (Header[15]<< 8) | Header[14];

        file.close();
    }
    return false;
}
