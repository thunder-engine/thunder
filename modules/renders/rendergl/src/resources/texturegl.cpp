#include "resources/texturegl.h"

#include <cstring>

#include "agl.h"
#include "commandbuffergl.h"

TextureGL::TextureGL() :
        m_id(0) {

}

uint32_t TextureGL::nativeHandle() {
    switch(state()) {
        case Unloading: {
            destroyTexture();
            switchState(ToBeDeleted);
        } break;
        case ToBeUpdated: {
            updateTexture();
            switchState(Ready);
        } break;
        default: break;
    }

    return m_id;
}

void TextureGL::readPixels(int x, int y, int width, int height) {
    bool depth = (format() == Depth);

    Sides *sides = getSides();
    if(!sides->empty()) {
        Surface &surface = sides->at(0);

        glReadPixels(x, y, width, height,
                     (depth) ? GL_DEPTH_COMPONENT : GL_RGBA,
                     (depth) ? GL_FLOAT : GL_UNSIGNED_BYTE, &(surface[0])[0]);
        CheckGLError();
    }
}

void TextureGL::updateTexture() {
    bool newObject = false;
    if(m_id == 0) {
        glGenTextures(1, &m_id);
        newObject = true;
    }

    uint32_t target = isCubemap() ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;
    glBindTexture(target, m_id);

    Texture::Sides *sides = getSides();

    bool mipmap = (sides->empty()) ? false : (sides->at(0).size() > 1);

    int32_t min = (mipmap) ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;
    int32_t mag = GL_NEAREST;

    switch(filtering()) {
        case Bilinear:  mag = GL_LINEAR; min = (mipmap) ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR; break;
        case Trilinear: mag = GL_LINEAR; min = GL_LINEAR_MIPMAP_LINEAR; break;
        default: break;
    }
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, min);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, mag);

    int32_t glwrap = GL_CLAMP_TO_EDGE;

    switch(wrap()) {
        case Repeat: glwrap = GL_REPEAT; break;
        case Mirrored: glwrap = GL_MIRRORED_REPEAT; break;
        default: break;
    }
    glTexParameteri(target, GL_TEXTURE_WRAP_S, glwrap);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, glwrap);
    glTexParameteri(target, GL_TEXTURE_WRAP_R, glwrap);

    uint32_t internal = GL_RGBA8;
    uint32_t glformat = GL_RGBA;
    uint32_t type     = GL_UNSIGNED_BYTE;

    switch(format()) {
        case R8: {
            internal = GL_R8;
            glformat = GL_RED;
        } break;
        case RGB8: {
            internal = GL_RGB8;
            glformat = GL_RGB;
        } break;
        case RGB10A2: {
    #ifndef THUNDER_MOBILE
            internal = GL_RGB10_A2;
            type     = GL_UNSIGNED_INT_10_10_10_2;
    #else
            internal = GL_RGB10_A2;
            type     = GL_UNSIGNED_INT_2_10_10_10_REV;
    #endif
        } break;
        case RGBA32Float: {
            internal = GL_RGBA32F;
            glformat = GL_RGBA;
            type     = GL_FLOAT;
        } break;
        case R11G11B10Float: {
            internal = GL_R11F_G11F_B10F;
            glformat = GL_RGB;
            type     = GL_FLOAT;
        } break;
        case Depth: {
            internal = (depthBits() == 16) ? GL_DEPTH_COMPONENT16 : GL_DEPTH_COMPONENT24;
            glformat = GL_DEPTH_COMPONENT;
            type     = GL_UNSIGNED_INT;
        } break;
        default: break;
    }

    switch(target) {
        case GL_TEXTURE_CUBE_MAP: {
            uploadTextureCubemap(sides, target, internal, glformat, type);
        } break;
        default: {
            uploadTexture(sides, 0, target, internal, glformat, type);
        } break;
    }

    if(newObject && !name().empty()) {
        CommandBufferGL::setObjectName(GL_TEXTURE, m_id, name());
    }

    //glTexParameterf(target, GL_TEXTURE_LOD_BIAS, 0.0);

    //float aniso = 0.0f;
    //glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
    //glTexParameterf(target, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
}

void TextureGL::destroyTexture() {
    if(m_id) {
        glDeleteTextures(1, &m_id);
        CheckGLError();
        m_id = 0;
    }
}

bool TextureGL::uploadTexture(const Sides *sides, uint32_t imageIndex, uint32_t target, uint32_t internal, uint32_t format, uint32_t type) {
    int32_t w = width();
    int32_t h = height();

    if(sides->empty()) {
        glTexImage2D(target, 0, internal, w, h, 0, format, type, nullptr);
    } else {
        const Surface &image = sides->at(imageIndex);
        if(isCompressed()) {
            // load all mipmaps
            for(uint32_t i = 0; i < image.size(); i++) {
                const int8_t *data = image[i].data();
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

            // load all mipmaps
            for(uint32_t i = 0; i < image.size(); i++) {
                const int8_t *data = image[i].data();
                glTexImage2D(target, i, internal, (w >> i), (h >> i), 0, format, type, data);
                CheckGLError();
            }
            if(alignment != -1) {
                glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
                CheckGLError();
            }
        }
    }

    return true;
}

bool TextureGL::uploadTextureCubemap(const Sides *sides, uint32_t target, uint32_t internal, uint32_t format, uint32_t type) {
    // loop through cubemap faces and load them as 2D textures
    for(uint32_t n = 0; n < 6; n++) {
        // specify cubemap face
        target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + n;
        if(!uploadTexture(sides, n, target, internal, format, type)) {
            return false;
        }
    }
    return true;
}
