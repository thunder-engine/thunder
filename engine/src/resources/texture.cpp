#include "resources/texture.h"

#include <avariant.h>

#define HEADER  "Header"

Texture::Texture() {
    clear();
}

Texture::~Texture() {
    clear();
}

void Texture::loadUserData(const AVariantMap &data) {
    clear();
    {
        auto it = data.find(HEADER);
        if(it != data.end()) {
            AVariantList header   = (*it).second.value<AVariantList>();

            auto i      = header.begin();
            m_Width     = (*i).toInt();
            i++;
            m_Height    = (*i).toInt();
            i++;
            //Reserved
            i++;

            m_type          = TextureType((*i).toInt());
            i++;
            m_components    = (*i).toInt();
            i++;
            m_format        = (*i).toInt();
            i++;
            m_filtering     = FilteringType((*i).toInt());
            i++;
            m_wrap          = WrapType((*i).toInt());
            i++;
        }
    }
}

void Texture::clear() {
    m_components    = 0;
    m_format        = 0;
    m_type          = Flat;
    m_filtering     = None;
    m_wrap          = Clamp;

    m_Width         = 1;
    m_Height        = 1;
}

uint32_t Texture::width() const {
    return m_Width;
}

uint32_t Texture::height() const {
    return m_Height;
}
