#include "resources/rendertexturegl.h"

#include "agl.h"

RenderTextureGL::RenderTextureGL() :
        m_Buffer(0),
        m_ID(0) {
    setWidth(1);
    setHeight(1);
}

void *RenderTextureGL::nativeHandle() {
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

void RenderTextureGL::updateTexture() {
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
    uint32_t glformat   = GL_RGBA;
    uint32_t type       = GL_UNSIGNED_BYTE;

    switch(format()) {
        case R8: {
            internal    = GL_R8;
            glformat    = GL_RED;
        } break;
        case RGB8: {
            internal    = GL_RGB8;
            glformat    = GL_RGB;
        } break;
        case RGB10A2: {
#ifndef THUNDER_MOBILE
            internal    = GL_RGB10_A2;
            type        = GL_UNSIGNED_INT_10_10_10_2;
#else
            internal    = GL_RGB10_A2;
            type        = GL_UNSIGNED_INT_2_10_10_10_REV;
#endif
        } break;
        case RGB16Float: {
            internal    = GL_RGB16F;
            glformat    = GL_RGB;
            type        = GL_FLOAT;
        } break;
        case R11G11B10Float: {
            internal    = GL_R11F_G11F_B10F;
            glformat    = GL_RGB;
            type        = GL_FLOAT;
        } break;
        default: break;
    }

    uint8_t depthBits = depth();
    if(depthBits) {
        glformat= GL_DEPTH_COMPONENT;
        type    = GL_UNSIGNED_INT;

        switch(depthBits) {
            case 16: {
                internal    = GL_DEPTH_COMPONENT16;
            } break;
            case 24: {
                internal    = GL_DEPTH_COMPONENT24;
            } break;
            default: break;
        }
    }

    if(target == GL_TEXTURE_CUBE_MAP) {
        for(int i = 0; i < 6; i++) {
            glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internal, width(), height(), 0, glformat, type, nullptr );
        }
    } else {
        glTexImage2D    ( target, 0, internal, width(), height(), 0, glformat, type, nullptr );
    }
}

void RenderTextureGL::destroyTexture() {
    if(m_Buffer) {
        glDeleteFramebuffers(1, &m_Buffer);
    }
    m_Buffer    = 0;

    if(m_ID) {
        glDeleteTextures(1, &m_ID);
    }
    m_ID = 0;
}

void RenderTextureGL::makeCurrent(uint32_t index) const {
    if(index == 0) {
        glBindFramebuffer(GL_FRAMEBUFFER, m_Buffer);
    }
    glFramebufferTexture2D( GL_FRAMEBUFFER, (depth()) ? GL_DEPTH_ATTACHMENT : GL_COLOR_ATTACHMENT0 + index, GL_TEXTURE_2D, m_ID, 0 );
}
