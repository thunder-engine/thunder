#include "resources/atexturegl.h"

#if __linux__
#include <cstring>
#endif

#define DATA    "Data"


ATextureGL::ATextureGL() :
        m_Buffer(0) {

    destroy();
}


ATextureGL::~ATextureGL() {
    destroy();
}

void ATextureGL::create(uint32_t target, uint32_t internal, uint32_t format, uint32_t bits) {
    destroy();

    m_Target    = target;
    m_Internal  = internal;
    m_GlFormat  = format;
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
            glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, m_Internal, m_Width, m_Height, 0, m_GlFormat, m_Bits, 0 );
        }
    } else {
        glTexImage2D    ( m_Target, 0, m_Internal, m_Width, m_Height, 0, m_GlFormat, m_Bits, 0 );
    }

    glGenFramebuffers(1, &m_Buffer);
}

void ATextureGL::destroy() {
    m_Target    = 0;
    m_GlFormat  = 0;
    mID         = 0;
    m_Bits      = GL_UNSIGNED_BYTE;
    m_Internal  = 0;

    if(m_Buffer) {
        glDeleteFramebuffers(1, &m_Buffer);
    }
}

void ATextureGL::resize(uint32_t width, uint32_t height) {
    m_Width     = width;
    m_Height    = height;

    glBindTexture       (m_Target, mID);
    if(m_Target == GL_TEXTURE_CUBE_MAP) {
        for(int i = 0; i < 6; i++) {
            glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, m_Internal, m_Width, m_Height, 0, m_GlFormat, m_Bits, 0 );
        }
    } else {
        glTexImage2D    ( m_Target, 0, m_Internal, m_Width, m_Height, 0, m_GlFormat, m_Bits, 0 );
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

    if(mID) {
        glDeleteTextures(1, &mID);
        mID = 0;
    }
}

void ATextureGL::apply() {
    Texture::apply();

    if(m_Sides.empty()) {
        return;
    }

    bool update = false;
    if(!mID) {
        glGenTextures(1, &mID);
    } else {
        update  = true;
    }

    m_Target    = (isCubemap()) ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;
    switch (m_Format) {
#if !(GL_ES_VERSION_2_0)
        case LUMINANCE: m_GlFormat  = GL_RED; break;
#else
        case LUMINANCE: m_GlFormat  = GL_RED_EXT; break;
#endif
        case RGB:       m_GlFormat  = GL_RGB; break;
        //case DXT1:      m_GlFormat  = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT; break;
        //case DXT5:      m_GlFormat  = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT; break;
        default:        m_GlFormat  = GL_RGBA; break;
    }

    glBindTexture(m_Target, mID);

    switch(m_Target) {
        case GL_TEXTURE_CUBE_MAP: {
            uploadTextureCubemap(m_Sides);
        } break;
        default: {
            uploadTexture2D(m_Sides, 0, m_Target, update);
        } break;
    }

    //glTexParameterf ( m_Target, GL_TEXTURE_LOD_BIAS, 0.0);

    bool mipmap = (m_Sides[0].size() > 1);

    uint32_t filtering;
    switch(m_Filtering) {
        case Bilinear:  filtering = (mipmap) ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR; break;
        case Trilinear: filtering = GL_LINEAR_MIPMAP_LINEAR; break;
        default: filtering  = (mipmap) ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST; break;
    }
    //glTexParameteri ( m_Target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri ( m_Target, GL_TEXTURE_MIN_FILTER, filtering );

    uint32_t wrap;
    switch (m_Wrap) {
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

bool ATextureGL::uploadTexture2D(const Sides &sides, uint32_t imageIndex, uint32_t target, bool update) {
    const Surface &image    = sides[imageIndex];

    if(isCompressed()) {
        // load all mipmaps
        uint32_t w  = m_Width;
        uint32_t h  = m_Height;
        for(uint32_t i = 0; i < image.size(); i++) {
            uint8_t *data   = image[i];
            glCompressedTexImage2D(target, i, m_GlFormat, w, h, 0, size(w, h), data);
            w   = MAX(w / 2, 1);
            h   = MAX(h / 2, 1);
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
            if(update) {
                glTexSubImage2D(target, i, 0, 0, w, h, m_GlFormat, GL_UNSIGNED_BYTE, data);
            } else {
                glTexImage2D(target, i, m_GlFormat, w, h, 0, m_GlFormat, GL_UNSIGNED_BYTE, data);
            }

            w   = MAX(w / 2, 1);
            h   = MAX(h / 2, 1);
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
    int dwordLineSize   = dwordAlignedLineSize(width(), components() * 8);
    int curLineSize     = width() * components();

    return (dwordLineSize == curLineSize);
}

inline uint32_t ATextureGL::dwordAlignedLineSize(uint32_t width, uint32_t bpp) {
    return ((width * bpp + 31) & -32) >> 3;
}
