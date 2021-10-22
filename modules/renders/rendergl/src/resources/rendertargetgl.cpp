#include "resources/rendertargetgl.h"

#include "agl.h"

#include "resources/texturegl.h"

RenderTargetGL::RenderTargetGL() :
        m_Buffer(-1) {

}

void RenderTargetGL::bindBuffer(uint32_t level) {
    switch(state()) {
        case Unloading: {
            destroyBuffer();
            setState(ToBeDeleted);
            return;
        } break;
        case ToBeUpdated: {
            if(updateBuffer(level)) {
                setState(Ready);
                return;
            }
        } break;
        default: break;
    }

    updateBuffer(level);
}

void RenderTargetGL::switchState(ResourceState state) {
    setState(state);
}

uint32_t RenderTargetGL::nativeHandle() const {
    return (uint32_t)m_Buffer;
}

void RenderTargetGL::setNativeHandle(uint32_t id) {
    if(m_Buffer == -1) {
        m_Buffer = id;
    }
}

bool RenderTargetGL::updateBuffer(uint32_t level) {
    if(m_Buffer == -1) {
        glGenFramebuffers(1, (GLuint *)&m_Buffer);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, m_Buffer);

    if(m_Buffer == 0) {
        return true;
    }

    uint32_t colors[8];
    uint32_t count = colorAttachmentCount();
    for(uint32_t i = 0; i < count; i++) {
        colors[i] = GL_COLOR_ATTACHMENT0 + i;
        TextureGL *c = static_cast<TextureGL *>(colorAttachment(i));
        if(c) {
            glFramebufferTexture2D(GL_FRAMEBUFFER, colors[i], GL_TEXTURE_2D, c->nativeHandle(), level);
        } else {
            // Set render buffer
        }
    }

    TextureGL *depth = static_cast<TextureGL *>(depthAttachment());
    if(depth) {
        uint32_t handle = depth->nativeHandle();
        if(handle > 0) {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, handle, level);
        } else {
            // Set render buffer
        }
    }

    if(count > 1) {
        glDrawBuffers(count, colors);
    }

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE) {
        return false;
    }
    return true;
}

void RenderTargetGL::destroyBuffer() {
    if(m_Buffer) {
        glDeleteFramebuffers(1, (GLuint *)&m_Buffer);
    }
    m_Buffer = -1;
}
