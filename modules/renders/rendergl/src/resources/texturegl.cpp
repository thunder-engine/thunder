#include "resources/texturegl.h"

#include <cstring>

#include "agl.h"

#define DATA    "Data"

TextureGL::TextureGL() :
        m_ID(0) {

}

void *TextureGL::nativeHandle() {
    switch(state()) {
        case Suspend: {
            destroyTexture();
            setState(ToBeDeleted);
        } break;
        case ToBeUpdated: {
            updateTexture();
            setState(Ready);
        } break;
        default: break;
    }

    return reinterpret_cast<void *>(m_ID);
}

void TextureGL::readPixels(int x, int y, int width, int height) {
    bool depth = (format() == Depth);

    Surface &surface = getSides()->at(0);

    glReadPixels(x, y, width, height,
                 (depth) ? GL_DEPTH_COMPONENT : GL_RGBA,
                 (depth) ? GL_FLOAT : GL_UNSIGNED_BYTE, &(surface[0])[0]);
    CheckGLError();
}

void TextureGL::updateTexture() {
    if(!m_ID) {
        glGenTextures(1, &m_ID);
        CheckGLError();
    }

    uint32_t target = GL_TEXTURE_2D;
    if(isCubemap()) {
        target = GL_TEXTURE_CUBE_MAP;
    }

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
        case RGB16Float: {
            internal    = GL_RGB16F;
            glformat    = GL_RGB;
        } break;
        case RGBA32Float: {
            internal    = GL_RGBA32F;
            glformat    = GL_RGBA;
        } break;
        default: break;
      //case DXT1:  format  = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT; break;
      //case DXT5:  format  = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT; break;
    }

    glBindTexture(target, m_ID);
    CheckGLError();

    switch(target) {
        case GL_TEXTURE_CUBE_MAP: {
            uploadTextureCubemap(getSides(), target, internal, glformat);
        } break;
        default: {
            uploadTexture(getSides(), 0, target, internal, glformat);
        } break;
    }
    //glTexParameterf ( target, GL_TEXTURE_LOD_BIAS, 0.0);

    bool mipmap = (getSides()->at(0).size() > 1);

    int32_t min = (mipmap) ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;
    int32_t mag = GL_NEAREST;
    switch(filtering()) {
        case Bilinear:  mag = GL_LINEAR; min = (mipmap) ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR; break;
        case Trilinear: mag = GL_LINEAR; min = GL_LINEAR_MIPMAP_LINEAR; break;
        default: break;
    }
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, min);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, mag);

    int32_t glwrap;
    switch(wrap()) {
        case Repeat: glwrap   = GL_REPEAT; break;
        case Mirrored: glwrap = GL_MIRRORED_REPEAT; break;
        default: glwrap       = GL_CLAMP_TO_EDGE; break;
    }
    glTexParameteri(target, GL_TEXTURE_WRAP_S, glwrap);
    CheckGLError();
    glTexParameteri(target, GL_TEXTURE_WRAP_T, glwrap);
    CheckGLError();
    glTexParameteri(target, GL_TEXTURE_WRAP_R, glwrap);
    CheckGLError();

    //float aniso = 0.0f;
    //glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
    //glTexParameterf ( target, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso );
}

void TextureGL::destroyTexture() {
    if(m_ID) {
        glDeleteTextures(1, &m_ID);
        CheckGLError();
        m_ID = 0;
    }
}

bool TextureGL::uploadTexture(const Sides *sides, uint32_t imageIndex, uint32_t target, uint32_t internal, uint32_t format, uint32_t index) {
    const Surface &image = (*sides)[imageIndex];

    if(isCompressed()) {
        // load all mipmaps
        int32_t w  = width();
        int32_t h  = height();
        for(uint32_t i = 0; i < image.size(); i++) {
            const int8_t *data = &(image[i])[0];
            glCompressedTexImage2D(target, i, internal, (w >> i), (h >> i), 0, size((w >> i), (h >> i)), data);
            CheckGLError();
        }
    } else {
        GLint alignment = -1;
        if(!isDwordAligned()) {
            glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);
            CheckGLError();
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            CheckGLError();
        }

        GLenum type = GL_UNSIGNED_BYTE;
        if(Texture::format() == RGB16Float || Texture::format() == RGBA32Float) {
            type = GL_FLOAT;
        }
        // load all mipmaps
        int32_t w = width();
        int32_t h = height();
        for(uint32_t i = 0; i < image.size(); i++) {
            const int8_t *data = &(image[i])[0];
            glTexImage2D(target, i, internal, (w >> i), (h >> i), 0, format, type, data);
            CheckGLError();
        }
        if(alignment != -1) {
            glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
            CheckGLError();
        }
    }

    return true;
}

bool TextureGL::uploadTextureCubemap(const Sides *sides, uint32_t target, uint32_t internal, uint32_t format) {
    // loop through cubemap faces and load them as 2D textures
    for(uint32_t n = 0; n < 6; n++) {
        // specify cubemap face
        target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + n;
        if(!uploadTexture(sides, n, target, internal, format)) {
            return false;
        }
    }
    return true;
}
