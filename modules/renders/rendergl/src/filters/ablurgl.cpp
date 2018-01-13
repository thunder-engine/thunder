#include "filters/ablurgl.h"

#include "engine.h"

#include "apipeline.h"

ABlurGL::ABlurGL(APipeline *pipeline) {
    m_pPipeline = pipeline;

    m_pBlurMaterial     = Engine::loadResource<AMaterialGL>(".embedded/Blur.frag");
    if(m_pBlurMaterial) {
        uint32_t program    = m_pBlurMaterial->getProgram(AMaterialGL::Static);

        u_Steps     = glGetUniformLocation(program, "steps");
        u_Size      = glGetUniformLocation(program, "size");
        u_Curve     = glGetUniformLocation(program, "curve");
        u_Direction = glGetUniformLocation(program, "direction");
    }
}

void ABlurGL::draw(ATextureGL &source, ATextureGL &target, ATextureGL &temp, Vector2 &size, uint8_t steps, float *points) {
    if(m_pBlurMaterial) {
        m_pBlurMaterial->bind(*m_pPipeline, IDrawObjectGL::UI, AMaterialGL::Static);
        int program = m_pBlurMaterial->getProgram(AMaterialGL::Static);
        if(u_Steps > -1) {
            glProgramUniform1i(program, u_Steps, steps);
        }
        if(u_Size > -1) {
            glProgramUniform2f(program, u_Size, 1.0f / size.x, 1.0f / size.y);
        }
        if(u_Curve > -1) {
            glProgramUniform1fv(program, u_Curve, MAX_SAMPLES, points);
        }
        if(u_Direction > -1) {
            glProgramUniform2f(program, u_Direction, 1.0f, 0.0f);
            m_pPipeline->drawScreen(source, temp);

            glEnable    ( GL_BLEND );
            glBlendFunc ( GL_ONE, GL_ONE );

            //m_pPipeline->clearScreen(target);
            glProgramUniform2f(program, u_Direction, 0.0f, 1.0f);
            m_pPipeline->drawScreen(temp, target);

            glDisable   ( GL_BLEND );
        }
        m_pBlurMaterial->unbind(IDrawObjectGL::UI);
    }
}
