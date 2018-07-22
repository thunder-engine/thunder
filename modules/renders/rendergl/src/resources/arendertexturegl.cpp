#include "resources/arendertexturegl.h"

ARenderTextureGL::~ARenderTextureGL() {
    clear();
}

void ARenderTextureGL::clear() {
    Texture::clear();

    if(m_Buffer) {
        glDeleteFramebuffers(1, &m_Buffer);
    }
    m_Buffer    = 0;

    if(m_ID) {
        glDeleteTextures(1, &m_ID);
    }
    m_ID = 0;
}

void ARenderTextureGL::apply() {
    if(!m_Buffer) {
        glGenFramebuffers(1, &m_Buffer);
    }

    if(!m_ID) {
        glGenTextures(1, &m_ID);
    }

    uint32_t target = (isCubemap()) ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;
    glBindTexture(target, m_ID);

    glTexParameterf ( target, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

    glTexParameterf ( target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameterf ( target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameterf ( target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );

    uint32_t internal   = GL_RGBA8;
    uint32_t format     = GL_RGBA;
    uint32_t type       = GL_UNSIGNED_BYTE;

    switch(m_Format) {
        case R8: {
            internal    = GL_R8;
            format      = GL_RED;
        } break;
        case RGB8: {
            internal    = GL_RGB8;
            format      = GL_RGB;
        } break;
        case RGB10A2: {
#ifndef THUNDER_MOBILE
            internal    = GL_RGB10_A2;
            type        = GL_UNSIGNED_INT_10_10_10_2;
#else
            //internal    = GL_RGB10_A2_EXT;
            //type        = GL_UNSIGNED_INT_10_10_10_2_OES;
#endif
            format      = GL_RGBA;
        } break;
        case R11G11B10Float: {
            internal    = GL_R11F_G11F_B10F;
            format      = GL_RGB;
            type        = GL_FLOAT;
        } break;
        default: break;
      //case DXT1:  format  = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT; break;
      //case DXT5:  format  = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT; break;
    }

    if(m_DepthBits) {
        format  = GL_DEPTH_COMPONENT;
        type    = GL_FLOAT;

        switch(m_DepthBits) {
            case 24: {
                internal    = GL_DEPTH_COMPONENT24;
            } break;
            default: break;
        }
    }

    if(target == GL_TEXTURE_CUBE_MAP) {
        for(int i = 0; i < 6; i++) {
            glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internal, m_Width, m_Height, 0, format, type, 0 );
        }
    } else {
        glTexImage2D    ( target, 0, internal, m_Width, m_Height, 0, format, type, 0 );
    }

}
