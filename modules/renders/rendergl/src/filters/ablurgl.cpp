#include "filters/ablurgl.h"

#include "engine.h"

#include "commandbuffergl.h"

ABlurGL::ABlurGL() {
    m_pBlurMaterial     = Engine::loadResource<AMaterialGL>(".embedded/Blur.frag");
    if(m_pBlurMaterial) {
/*
        uint32_t program    = m_pBlurMaterial->getProgram(AMaterialGL::Static);

        u_Steps     = glGetUniformLocation(program, "steps");
        u_Size      = glGetUniformLocation(program, "size");
        u_Curve     = glGetUniformLocation(program, "curve");
        u_Direction = glGetUniformLocation(program, "direction");
*/
    }
}

void ABlurGL::draw(ICommandBuffer &buffer, ATextureGL &source, ATextureGL &target, ATextureGL &temp, Vector2 &size, uint8_t steps, float *points) {
    if(m_pBlurMaterial && m_pBlurMaterial->bind(ICommandBuffer::UI)) {
        if(u_Steps > -1) {
            glUniform1i(u_Steps, steps);
        }
        if(u_Size > -1) {
            glUniform2f(u_Size, 1.0f / size.x, 1.0f / size.y);
        }
        if(u_Curve > -1) {
            glUniform1fv(u_Curve, MAX_SAMPLES, points);
        }
        if(u_Direction > -1) {
            glUniform2f(u_Direction, 1.0f, 0.0f);
            /// \todo Return command buffer
            //m_pPipeline->drawScreen(source, temp);
            //glBindFramebuffer       ( GL_FRAMEBUFFER, m_ScreenBuffer );
            //glFramebufferTexture2D  ( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target.id(), 0 );
            ////drawTexturedQuad(source);
            //glBindFramebuffer   ( GL_FRAMEBUFFER, 0 );

            glEnable    ( GL_BLEND );
            glBlendFunc ( GL_ONE, GL_ONE );

            glUniform2f(u_Direction, 0.0f, 1.0f);
            /// \todo Return command buffer
            //m_pPipeline->drawScreen(temp, target);

            glDisable   ( GL_BLEND );
        }
        m_pBlurMaterial->unbind(ICommandBuffer::UI);
    }
}
