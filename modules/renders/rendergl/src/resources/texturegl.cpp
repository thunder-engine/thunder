#include "resources/texturegl.h"

//#include <cstring>

#include "agl.h"
#include "commandbuffergl.h"

TextureGL::TextureGL() :
        m_id(0) {

}

uint32_t TextureGL::nativeHandle() {
    switch(state()) {
        case ToBeUpdated: {
            updateTexture();
            switchState(Ready);
        } break;
        case Unloading: {
            destroyTexture();
            switchState(ToBeDeleted);
        } break;
        default: break;
    }

    return m_id;
}

void TextureGL::readPixels(int x, int y, int width, int height) {
    if(sides() != 0) {
        Surface &dst = surface(0);

        bool depth = (format() == Depth);
        glReadPixels(x, y, width, height,
                    (depth) ? GL_DEPTH_COMPONENT : GL_RGBA,
                    (depth) ? GL_FLOAT : GL_UNSIGNED_BYTE, dst[0].data());
        CheckGLError();
    }
}

void TextureGL::updateTexture() {
    bool newObject = false;
    if(m_id == 0) {
        glGenTextures(1, &m_id);
        newObject = true;
    }

    uint32_t target = GL_TEXTURE_2D;
    if(isCubemap()) {
        target = GL_TEXTURE_CUBE_MAP;
    }
    if(m_depth > 1) {
        target = GL_TEXTURE_3D;
    }

    glBindTexture(target, m_id);

    bool mipmap = mipCount() > 1;

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

    if(isCubemap()) {
        glwrap = GL_MIRRORED_REPEAT;
    } else {
        switch(wrap()) {
            case Repeat: glwrap = GL_REPEAT; break;
            case Mirrored: glwrap = GL_MIRRORED_REPEAT; break;
            default: break;
        }
    }

    glTexParameteri(target, GL_TEXTURE_WRAP_S, glwrap);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, glwrap);
    glTexParameteri(target, GL_TEXTURE_WRAP_R, glwrap);

    uint32_t internal = GL_RGBA8;
    uint32_t glformat = GL_RGBA;
    uint32_t type     = GL_UNSIGNED_BYTE;

    if(m_compress == Uncompressed) {
        switch(m_format) {
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
            case R11G11B10Float: {
                internal = GL_R11F_G11F_B10F;
                glformat = GL_RGB;
                type     = GL_FLOAT;
            } break;
            case RGBA32Float: {
                internal = GL_RGBA32F;
                glformat = GL_RGBA;
                type     = GL_FLOAT;
            } break;
            case RGBA16Float: {
                internal = GL_RGBA16F;
                glformat = GL_RGBA;
                type     = GL_FLOAT;
            } break;
            case Depth: {
                internal = (m_depthBits == 16) ? GL_DEPTH_COMPONENT16 : GL_DEPTH_COMPONENT24;
                glformat = GL_DEPTH_COMPONENT;
                type     = GL_UNSIGNED_INT;
            } break;
            default: break;
        }
    } else {
        switch(m_compress) {
#ifndef THUNDER_MOBILE
            case BC1: internal = GL_COMPRESSED_RGB_S3TC_DXT1_EXT; break;
            case BC3: internal = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT; break;
            case BC7: internal = GL_COMPRESSED_RGBA_BPTC_UNORM_ARB; break;
#endif
            case ASTC: internal = GL_COMPRESSED_RGBA_ASTC_4x4_KHR; break;
            case ETC1: internal = GL_COMPRESSED_RGB8_ETC2; break;
            case ETC2: internal = GL_COMPRESSED_RGBA8_ETC2_EAC; break;
            default: break;
        }
    }

    if(target == GL_TEXTURE_CUBE_MAP) {
        uploadTextureCubemap(target, internal, glformat, type);
    } else {
        uploadTexture(0, target, internal, glformat, type);
    }
#ifndef THUNDER_MOBILE
    if(newObject && !name().isEmpty()) {
        CommandBufferGL::setObjectName(GL_TEXTURE, m_id, name());
    }
#endif
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

bool TextureGL::uploadTexture(uint32_t imageIndex, uint32_t target, uint32_t internal, uint32_t format, uint32_t type) {
    if(isRender()) {
        glTexImage2D(target, 0, internal, m_width, m_height, 0, format, type, nullptr);
    } else {
        const Surface &image = surface(imageIndex);
        if(m_compress != Uncompressed) {
            // load all mipmaps
            for(uint32_t i = 0; i < image.size(); i++) {
                glCompressedTexImage2D(target, i, internal, (m_width >> i), (m_height >> i), 0, image[i].size(), image[i].data());
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
                if(m_depth > 1) {
                    glTexImage3D(target, i, internal, (m_width >> i), (m_height >> i), (m_depth >> i), 0, format, type, image[i].data());
                } else {
                    glTexImage2D(target, i, internal, (m_width >> i), (m_height >> i), 0, format, type, image[i].data());
                }
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

bool TextureGL::uploadTextureCubemap(uint32_t target, uint32_t internal, uint32_t format, uint32_t type) {
    // loop through cubemap faces and load them as 2D textures
    for(uint32_t n = 0; n < 6; n++) {
        // specify cubemap face
        target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + n;
        if(!uploadTexture(n, target, internal, format, type)) {
            return false;
        }
    }
    return true;
}
