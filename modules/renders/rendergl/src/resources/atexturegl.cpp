#include "resources/atexturegl.h"

#define DATA    "Data"

ATextureGL::ATextureGL() {
    m_Target    = 0;
    m_Format    = 0;
    mID         = 0;
    m_Bits      = GL_UNSIGNED_BYTE;
    mInternal   = 0;
}

void ATextureGL::create(uint32_t target, uint32_t internal, uint32_t format, uint32_t bits) {
    m_Target    = target;
    mInternal   = internal;
    m_Format    = format;
    m_Bits      = bits;

    glGenTextures   ( 1, &mID );
    glBindTexture   ( m_Target, mID );

    glTexParameterf ( m_Target, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

    glTexParameterf ( m_Target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameterf ( m_Target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    if(target == GL_TEXTURE_CUBE_MAP) {
#if !(GL_ES_VERSION_2_0)
        glTexParameterf ( target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
#endif
        for(int i = 0; i < 6; i++) {
            glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, mInternal, m_Width, m_Height, 0, m_Format, m_Bits, 0 );
        }
    } else {
        glTexImage2D    ( m_Target, 0, mInternal, m_Width, m_Height, 0, m_Format, m_Bits, 0 );
    }
}

void ATextureGL::resize(uint32_t width, uint32_t height) {
    m_Width     = width;
    m_Height    = height;

    glBindTexture       (m_Target, mID);
    if(m_Target == GL_TEXTURE_CUBE_MAP) {
        for(int i = 0; i < 6; i++) {
            glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, mInternal, m_Width, m_Height, 0, m_Format, m_Bits, 0 );
        }
    } else {
        glTexImage2D    ( m_Target, 0, mInternal, m_Width, m_Height, 0, m_Format, m_Bits, 0 );
    }
}

void ATextureGL::bind() const {
    glBindTexture   (m_Target, mID);
}

void ATextureGL::unbind() const {
    glBindTexture   (m_Target, 0);
}

void ATextureGL::clear() {
    Texture::clear();

    glDeleteTextures(1, &mID);
}

uint32_t ATextureGL::size(uint32_t width, uint32_t height) const {
    uint32_t (ATextureGL::*sizefunc)(uint32_t, uint32_t) const;
    sizefunc    = (isCompressed() ? &ATextureGL::sizeDXTc : &ATextureGL::sizeRGB);

    return (this->*sizefunc)(width, height);
}

inline uint32_t ATextureGL::sizeDXTc(uint32_t width, uint32_t height) const {
    return ((width + 3) / 4) * ((height + 3) / 4) * (m_format == DXT1 ? 8 : 16);
}

inline uint32_t ATextureGL::sizeRGB(uint32_t width, uint32_t height) const {
    return width * height * m_components;
}

void ATextureGL::loadUserData(const VariantMap &data) {
    Texture::loadUserData(data);

    Sides sides;
    {
        auto it = data.find(DATA);
        if(it != data.end()) {
            const VariantList &surfaces = (*it).second.value<VariantList>();
            for(auto s : surfaces) {
                Surface img;
                uint32_t w  = m_Width;
                uint32_t h  = m_Height;
                const VariantList &lods = s.value<VariantList>();
                for(auto l : lods) {
                    ByteArray bits = l.toByteArray();
                    uint32_t s  = size(w, h);
                    if(s && !bits.empty()) {
                        uint8_t *pixels = new uint8_t[s];
                        memcpy(pixels, &bits[0], s);
                        img.push_back(pixels);
                    }
                    w   = MAX(w / 2, 1);
                    h   = MAX(h / 2, 1);
                }
                sides.push_back(img);
            }
        }
    }

    glGenTextures(1, &mID);

    m_Target    = (isCubemap()) ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;
    switch (m_format) {
        case RGB:       m_Format    = GL_RGB; break;
        //case DXT1:      m_Format    = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT; break;
        //case DXT5:      m_Format    = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT; break;
        default:        m_Format    = GL_RGBA; break;
    }

    glBindTexture(m_Target, mID);

    switch(m_Target) {
        case GL_TEXTURE_CUBE_MAP: {
            uploadTextureCubemap(sides);
        } break;
        default: {
            uploadTexture2D(sides);
        } break;
    }

    //glTexParameterf ( m_Target, GL_TEXTURE_LOD_BIAS, 0.0);

    bool mipmap = (sides[0].size() > 1);

    uint32_t filtering;
    switch(m_filtering) {
        case Bilinear:  filtering = (mipmap) ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR; break;
        case Trilinear: filtering = GL_LINEAR_MIPMAP_LINEAR; break;
        default: filtering  = (mipmap) ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST; break;
    }
    //glTexParameteri ( m_Target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri ( m_Target, GL_TEXTURE_MIN_FILTER, filtering );

    uint32_t wrap;
    switch (m_wrap) {
        case Repeat: wrap   = GL_REPEAT; break;
        case Mirrored: wrap = GL_MIRRORED_REPEAT; break;
        default: wrap       = GL_CLAMP_TO_EDGE; break;
    }
    glTexParameteri ( m_Target, GL_TEXTURE_WRAP_S, wrap );
    glTexParameteri ( m_Target, GL_TEXTURE_WRAP_T, wrap );
    //glTexParameteri ( m_Target, GL_TEXTURE_WRAP_R, wrap );
/*
    float aniso = 0.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
    glTexParameterf ( m_Target, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso );
*/
    glBindTexture(m_Target, 0);
}

bool ATextureGL::uploadTexture2D(const Sides &sides, uint32_t imageIndex, uint32_t target) {
    const Surface &image    = sides[imageIndex];

    if(isCompressed()) {
        // load all mipmaps
        uint32_t w  = m_Width;
        uint32_t h  = m_Height;
        for(uint32_t i = 0; i < image.size(); i++) {
            uint8_t *data   = image[i];
            glCompressedTexImage2D(target, i, m_Format, w, h, 0, size(w, h), data);
            w   = MAX(w / 2, 1);
            h   = MAX(h / 2, 1);
            delete []data;
        }
    } else {
        GLint alignment = -1;
        if(!isDwordAligned()) {
            glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        }
        // load all mipmaps
        uint32_t w  = m_Width;
        uint32_t h  = m_Height;
        for(uint32_t i = 0; i < image.size(); i++) {
            uint8_t *data   = image[i];
            glTexImage2D(target, i, (m_components == 3) ?  GL_RGB : GL_RGBA, w, h, 0, m_Format, GL_UNSIGNED_BYTE, data);
            w   = MAX(w / 2, 1);
            h   = MAX(h / 2, 1);
            delete []data;
        }
        if(alignment != -1) {
            glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
        }
    }

    return true;
}

bool ATextureGL::uploadTextureCubemap(const Sides &sides) {
    GLenum target;
    // loop through cubemap faces and load them as 2D textures
    for(uint32_t n = 0; n < 6; n++) {
        // specify cubemap face
        target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + n;
        if(!uploadTexture2D(sides, n, target)) {
            return false;
        }
    }

    return true;
}

bool ATextureGL::isDwordAligned() {
    int dwordLineSize   = dwordAlignedLineSize(width(), m_components*8);
    int curLineSize     = width() * m_components;

    return (dwordLineSize == curLineSize);
}

inline uint32_t ATextureGL::dwordAlignedLineSize(uint32_t width, uint32_t bpp) {
    return ((width * bpp + 31) & -32) >> 3;
}
