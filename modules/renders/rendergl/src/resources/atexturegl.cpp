#include "resources/atexturegl.h"

#include <cstring>

#define DATA    "Data"

ATextureGL::ATextureGL() :
    m_ID(0) {

}

void ATextureGL::clear() {
    Texture::clear();

    if(m_ID) {
        glDeleteTextures(1, &m_ID);
        CheckGLError();
        m_ID = 0;
    }
}

void ATextureGL::apply() {
    Texture::apply();

    if(getSides()->empty()) {
        return;
    }

    bool update = false;
    if(!m_ID) {
        glGenTextures(1, &m_ID);
        CheckGLError();
    } else {
        update  = true;
    }

    uint32_t target = (isCubemap()) ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;
    uint32_t internal   = GL_RGBA8;
    uint32_t glformat   = GL_RGBA;

    switch(format()) {
        case R8: {
            internal    = GL_R8;
            glformat    = GL_RED;
        } break;
        case RGB8: {
            internal    = GL_RGB8;
            glformat    = GL_RGB;
        } break;
        default: break;
      //case DXT1:  format  = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT; break;
      //case DXT5:  format  = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT; break;
    }

    glBindTexture(target, m_ID);
    CheckGLError();

    switch(target) {
        case GL_TEXTURE_CUBE_MAP: {
            uploadTextureCubemap(getSides(), internal, glformat);
        } break;
        default: {
            uploadTexture2D(getSides(), 0, target, internal, glformat, update);
        } break;
    }

    //glTexParameterf ( target, GL_TEXTURE_LOD_BIAS, 0.0);

    bool mipmap = (getSides()->at(0).size() > 1);

    int32_t glfiltering;
    switch(filtering()) {
        case Bilinear:  glfiltering = (mipmap) ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR; break;
        case Trilinear: glfiltering = GL_LINEAR_MIPMAP_LINEAR; break;
        default: glfiltering  = (mipmap) ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST; break;
    }
    //glTexParameteri ( target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri ( target, GL_TEXTURE_MIN_FILTER, glfiltering );

    int32_t glwrap;
    switch (wrap()) {
        case Repeat: glwrap   = GL_REPEAT; break;
        case Mirrored: glwrap = GL_MIRRORED_REPEAT; break;
        default: glwrap       = GL_CLAMP_TO_EDGE; break;
    }
    glTexParameteri ( target, GL_TEXTURE_WRAP_S, glwrap );
    CheckGLError();
    glTexParameteri ( target, GL_TEXTURE_WRAP_T, glwrap );
    CheckGLError();
    glTexParameteri ( target, GL_TEXTURE_WRAP_R, glwrap );
    CheckGLError();

    //float aniso = 0.0f;
    //glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
    //glTexParameterf ( target, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso );

    glBindTexture(target, 0);
}

void ATextureGL::readPixels(int32_t x, int32_t y, int32_t width, int32_t height) {
    bool depth = (format() == Depth);

    Surface &surface = getSides()->at(0);

    glReadPixels( x, y, width, height,
                 (depth) ? GL_DEPTH_COMPONENT : GL_RGBA,
                 (depth) ? GL_FLOAT : GL_UNSIGNED_BYTE, surface[0]);
    CheckGLError();
}

bool ATextureGL::uploadTexture2D(const Sides *sides, uint32_t imageIndex, uint32_t target, uint32_t internal, uint32_t format, bool update) {
    const Surface &image = (*sides)[imageIndex];

    if(isCompressed()) {
        // load all mipmaps
        int32_t w  = width();
        int32_t h  = height();
        for(uint32_t i = 0; i < image.size(); i++) {
            uint8_t *data   = image[i];
            glCompressedTexImage2D(target, i, internal, w, h, 0, size(w, h), data);
            CheckGLError();
            w   = MAX(w / 2, 1);
            h   = MAX(h / 2, 1);
        }
    } else {
        GLint alignment = -1;
        if(!isDwordAligned()) {
            glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);
            CheckGLError();
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            CheckGLError();
        }
        // load all mipmaps
        int32_t w  = width();
        int32_t h  = height();
        for(uint32_t i = 0; i < image.size(); i++) {
            uint8_t *data   = image[i];
            if(update) {
                glTexSubImage2D(target, i, 0, 0, w, h, format, GL_UNSIGNED_BYTE, data);
                CheckGLError();
            } else {
                glTexImage2D(target, i, internal, w, h, 0, format, GL_UNSIGNED_BYTE, data);
                CheckGLError();
            }

            w   = MAX(w / 2, 1);
            h   = MAX(h / 2, 1);
        }
        if(alignment != -1) {
            glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
            CheckGLError();
        }
    }

    return true;
}

bool ATextureGL::uploadTextureCubemap(const Sides *sides, uint32_t internal, uint32_t format) {
    GLenum target;
    // loop through cubemap faces and load them as 2D textures
    for(uint32_t n = 0; n < 6; n++) {
        // specify cubemap face
        target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + n;
        if(!uploadTexture2D(sides, n, target, internal, format)) {
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

inline int32_t ATextureGL::dwordAlignedLineSize(int32_t width, int32_t bpp) {
    return ((width * bpp + 31) & -32) >> 3;
}
