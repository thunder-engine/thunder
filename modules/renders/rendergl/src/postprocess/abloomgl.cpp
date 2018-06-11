#include "postprocess/abloomgl.h"

#include "engine.h"

#include "pipeline.h"

#include <cmath>
#if __linux__
#include <cstring>
#endif

ABloomGL::ABloomGL() {
    reset("shaders/Downsample.frag");
/*
    uint32_t format  = GL_R11F_G11F_B10F;

    for(uint8_t i = 0; i < BLOOM_PASSES; i++) {
        m_BloomPasses[i].DownTexture.create(GL_TEXTURE_2D, format, GL_RGB, GL_FLOAT);
    }
    m_BlurTemp.create(GL_TEXTURE_2D, format, GL_RGB, GL_FLOAT);
*/
    m_Threshold = 1.0f;

    m_BloomPasses[0].BlurSize   = Vector3(1.0f,  0.0f,  4.0f);
    m_BloomPasses[1].BlurSize   = Vector3(4.0f,  0.0f,  8.0f);
    m_BloomPasses[2].BlurSize   = Vector3(16.0f, 0.0f, 16.0f);
    m_BloomPasses[3].BlurSize   = Vector3(32.0f, 0.0f, 32.0f);
    m_BloomPasses[4].BlurSize   = Vector3(64.0f, 0.0f, 64.0f);
}

RenderTexture *ABloomGL::draw(RenderTexture &source, ICommandBuffer &buffer) {
    if(m_pMaterial) {
        buffer.setScreenProjection();
        //m_pMaterial->bind(buffer, nullptr, ICommandBuffer::UI, AMaterialGL::Static)
        //uint32_t program    = m_pMaterial->getProgram(AMaterialGL::Static);
        /// \todo Return command buffer
        //buffer.setShaderParams(program);

        //int location    = glGetUniformLocation(program, "threshold");
        //for(uint8_t i = 0; i < BLOOM_PASSES; i++) {
        //    if(location > -1) {
        //        glUniform1f(location, (i == 0) ? m_Threshold : 0.0f);
        //    }
        //    /// \todo Return command buffer
        //    //buffer.drawScreen((i == 0) ? source : m_BloomPasses[i - 1].DownTexture, m_BloomPasses[i].DownTexture);
        //    //glBindFramebuffer       ( GL_FRAMEBUFFER, m_ScreenBuffer );
        //    //glFramebufferTexture2D  ( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target.id(), 0 );
        //    ////drawTexturedQuad(source);
        //    //glBindFramebuffer   ( GL_FRAMEBUFFER, 0 );
        //}

        //pipeline.clearScreen(m_ResultTexture);
/*
        glBindFramebuffer       ( GL_FRAMEBUFFER, m_ScreenBuffer );
        glFramebufferTexture2D  ( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target.id(), 0 );

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear     (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindFramebuffer   ( GL_FRAMEBUFFER, 0 );
*/
        /// \todo Return command buffer
        //ABlurGL *blur   = pipeline.filterBlur();
        //for(uint8_t i = 0; i < BLOOM_PASSES; i++) {
        //    Vector2 size(m_BloomPasses[i].DownTexture.width(), m_BloomPasses[i].DownTexture.height());
        //    m_BlurTemp.resize(size.x, size.y);
        //
        //    blur->draw(m_BloomPasses[i].DownTexture, m_ResultTexture, m_BlurTemp, size, m_BloomPasses[i].BlurSteps, m_BloomPasses[i].BlurPoints);
        //}
        return m_pResultTexture;
    }

    return &source;
}

void ABloomGL::resize(int32_t width, int32_t height) {
    APostProcessor::resize(width, height);

    uint8_t div = 2;
    for(uint8_t i = 0; i < BLOOM_PASSES; i++) {
        uint32_t size   = width / div;
        float radius    = size * (m_BloomPasses[i].BlurSize.x * 1.0f) * 2 * 0.01f;

        m_BloomPasses[i].DownTexture.resize(size, height / div);
        m_BloomPasses[i].BlurSteps  = CLAMP((int)radius, 0, MAX_SAMPLES);

        memset(m_BloomPasses[i].BlurPoints, 0, sizeof(float) * MAX_SAMPLES);

        float total = 0.0f;
        for(uint8_t p = 0; p < m_BloomPasses[i].BlurSteps; p++) {
            float weight    = std::exp(-(float)(p * p) / (2.0f * radius));
            m_BloomPasses[i].BlurPoints[p]  = weight;

            total += weight;
        }

        for(uint8_t p = 0; p < m_BloomPasses[i].BlurSteps; p++) {
            m_BloomPasses[i].BlurPoints[p]  *= 1.0f / total;// 1.0 / (sqrt(2.0 * PI) * sigma;
        }

        div *= 2;
    }
}
