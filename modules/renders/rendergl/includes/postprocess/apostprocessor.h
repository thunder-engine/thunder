#ifndef APOSTPROCESSOR_H
#define APOSTPROCESSOR_H

#include <stdint.h>

#include "resources/amaterialgl.h"
#include "resources/atexturegl.h"

#include "commandbuffergl.h"

class APostProcessor {
public:
    virtual ~APostProcessor     () {

    }

    virtual ATextureGL         *draw                (ATextureGL &source, ICommandBuffer &buffer) {
        if(m_pMaterial) {
            buffer.setViewProjection(Matrix4(), Matrix4::ortho( 0.5f,-0.5f,-0.5f, 0.5f, 0.0f, 1.0f));
            m_pMaterial->bind(buffer, nullptr, ICommandBuffer::UI, AMaterialGL::Static);
            /// \todo Return command buffer
            //pipeline.drawScreen(source, m_ResultTexture);
            //glBindFramebuffer       ( GL_FRAMEBUFFER, m_ScreenBuffer );
            //glFramebufferTexture2D  ( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target.id(), 0 );
            ////drawTexturedQuad(source);
            //glBindFramebuffer   ( GL_FRAMEBUFFER, 0 );
            return &m_ResultTexture;
        }
        return &source;
    }

    virtual void                resize              (uint32_t width, uint32_t height) {
        m_ResultTexture.resize(width, height);
    }

    virtual void                reset               (const string &path) {
        m_pMaterial = Engine::loadResource<AMaterialGL>(path);

#if defined(GL_ES_VERSION_2_0) && !defined(GL_ES_VERSION_3_0)
        uint32_t format  = GL_R11F_G11F_B10F_APPLE;
#else
        uint32_t format  = GL_R11F_G11F_B10F;
#endif

        m_ResultTexture.create(GL_TEXTURE_2D, format, GL_RGB, GL_FLOAT);
    }

protected:
    ATextureGL                  m_ResultTexture;

    AMaterialGL                *m_pMaterial;
};

#endif // APOSTPROCESS_H
