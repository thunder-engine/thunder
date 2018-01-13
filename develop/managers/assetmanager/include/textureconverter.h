#ifndef TEXTURECONVERTER_H
#define TEXTURECONVERTER_H

#include "resources/texture.h"

#include "baseconvertersettings.h"

class QImage;

class TextureSerial : public Texture {
public:
    AVariantList                m_Surfaces;

    void                        loadUserData                (const AVariantMap &data);

    AVariantMap                 saveUserData                () const;
};

class TextureConverter : public IConverter {
public:
    TextureConverter            () {}

    string                      format                      () const { return "bmp;dds;jpg;jpeg;png;tga;ico;tif"; }
    IConverter::ContentTypes    type                        () const { return IConverter::ContentTexture; }
    uint8_t                     convertFile                 (IConverterSettings *);

    AVariantMap                 convertResource             (IConverterSettings *);

protected:
    bool                        tgaReader                   (IConverterSettings &settings, QImage &t);
};

#endif // TEXTURECONVERTER_H
