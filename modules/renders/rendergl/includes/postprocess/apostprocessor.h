#ifndef APOSTPROCESSOR_H
#define APOSTPROCESSOR_H

#include <stdint.h>

#include "resources/material.h"
#include "resources/rendertexture.h"

#include "commandbuffer.h"

class APostProcessor {
public:
    virtual ~APostProcessor     () {

    }

    virtual RenderTexture      *draw                (RenderTexture &source, ICommandBuffer &buffer) {
        if(m_pMaterial) {
            buffer.setScreenProjection();
            //m_pMaterial->bind(buffer, nullptr, ICommandBuffer::UI, AMaterialGL::Static);
            /// \todo Return command buffer
            //pipeline.drawScreen(source, m_ResultTexture);
            //glBindFramebuffer       ( GL_FRAMEBUFFER, m_ScreenBuffer );
            //glFramebufferTexture2D  ( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target.id(), 0 );
            //drawTexturedQuad(source);
            //glBindFramebuffer   ( GL_FRAMEBUFFER, 0 );
            return m_pResultTexture;
        }
        return &source;
    }

    virtual void                resize              (uint32_t width, uint32_t height) {
        m_pResultTexture->resize(width, height);
    }

    virtual void                reset               (const string &path) {
        m_pMaterial         = Engine::loadResource<Material>(path);
        m_pResultTexture    = Engine::objectCreate<RenderTexture>();

#if defined(GL_ES_VERSION_2_0) && !defined(GL_ES_VERSION_3_0)
        uint32_t format  = GL_R11F_G11F_B10F_APPLE;
#else
        //uint32_t format  = GL_R11F_G11F_B10F;
#endif

        //m_pResultTexture->setFormat(format);
    }

protected:
    RenderTexture              *m_pResultTexture;

    Material                   *m_pMaterial;
};

#endif // APOSTPROCESS_H
