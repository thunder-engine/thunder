#include "textureimportsettings.h"

#include <QVariantMap>

TextureImportSettings::TextureImportSettings(QObject *parent) :
        BaseConverterSettings(parent),
        m_TextureType(TextureType::Texture2D),
        m_FormType(Uncompressed_R8G8B8),
        m_Filtering(None),
        m_Wrap(Repeat),
        m_Lod(false) {

}

void TextureImportSettings::loadProperties(const QVariantMap &map) {
    auto it  = map.find("Type");
    if(it != map.end()) {
        setTextureType( TextureType(it.value().toInt()) );
    }

    it  = map.find("Format");
    if(it != map.end()) {
        setFormatType( FormatType(it.value().toInt()) );
    }

    it  = map.find("Wrap");
    if(it != map.end()) {
        setWrap( WrapType(it.value().toInt()) );
    }

    it  = map.find("MIP_maping");
    if(it != map.end()) {
        setLod( it.value().toBool() );
    }

    it  = map.find("Filtering");
    if(it != map.end()) {
        setFiltering( FilteringType(it.value().toInt()) );
    }

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
        m_Wrap = wrap;
        emit updated();
    }
}

bool TextureImportSettings::lod() const {
    return m_Lod;
}
void TextureImportSettings::setLod(bool lod) {
    if(m_Lod != lod) {
        m_Lod  = lod;
        emit updated();
    }
}
