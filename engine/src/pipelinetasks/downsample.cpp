#include "pipelinetasks/downsample.h"

#include "engine.h"

#include "resources/rendertarget.h"
#include "resources/material.h"

#include "pipelinecontext.h"
#include "commandbuffer.h"

#include "components/private/postprocessorsettings.h"

#include <cmath>
#include <cstring>

namespace {
    const char *rgbMap("rgbMap");
};

Downsample::Downsample() {

    setName("Downsample");

    Material *downSample = Engine::loadResource<Material>(".embedded/Downsample.shader");

    for(uint8_t i = 0; i < NUMBER_OF_PASSES; i++) {
        m_downPasses[i].downTexture = Engine::objectCreate<Texture>(std::string("downSampleTexture") + std::to_string(i));
        m_downPasses[i].downTexture->setFormat(Texture::R11G11B10Float);
        m_downPasses[i].downTexture->setFiltering(Texture::Bilinear);
        m_downPasses[i].downTexture->setFlags(Texture::Render);

        m_downPasses[i].downTarget = Engine::objectCreate<RenderTarget>("downSampleTarget");
        m_downPasses[i].downTarget->setColorAttachment(0, m_downPasses[i].downTexture);

        if(downSample) {
            m_downPasses[i].downMaterial = downSample->createInstance();
            if(i > 0) {
                m_downPasses[i].downMaterial->setTexture(rgbMap, m_downPasses[i - 1].downTexture);
            }
        }
    }

    m_inputs.push_back("In");
    m_outputs.push_back(std::make_pair("Result", nullptr));

    m_outputs.push_back(std::make_pair("Downsample 1/2", m_downPasses[0].downTexture));
    m_outputs.push_back(std::make_pair("Downsample 1/4", m_downPasses[1].downTexture));
    m_outputs.push_back(std::make_pair("Downsample 1/8", m_downPasses[2].downTexture));
    m_outputs.push_back(std::make_pair("Downsample 1/16", m_downPasses[3].downTexture));
    m_outputs.push_back(std::make_pair("Downsample 1/32", m_downPasses[4].downTexture));
}

Downsample::~Downsample() {
    for(int i = 0; i < NUMBER_OF_PASSES; i++) {
        m_downPasses[i].downTexture->deleteLater();
        m_downPasses[i].downTarget->deleteLater();

        delete m_downPasses[i].downMaterial;
    }
}

void Downsample::exec(PipelineContext &context) {
    CommandBuffer *buffer = context.buffer();

    buffer->beginDebugMarker("Downsample");

    for(uint8_t i = 0; i < NUMBER_OF_PASSES; i++) {
        buffer->setViewport(0, 0, m_downPasses[i].downTexture->width(), m_downPasses[i].downTexture->height());

        buffer->setRenderTarget(m_downPasses[i].downTarget);

        buffer->drawMesh(PipelineContext::defaultPlane(), 0, CommandBuffer::UI, *m_downPasses[i].downMaterial);
    }

    buffer->setViewport(0, 0, m_width, m_height);

    buffer->endDebugMarker();
}

void Downsample::resize(int32_t width, int32_t height) {
    if(m_width != width || m_height != height) {
        for(uint8_t i = 0; i < NUMBER_OF_PASSES; i++) {
            m_downPasses[i].downTexture->resize(MAX(width >> i, 1), MAX(height >> i, 1));
        }

        m_width = width;
        m_height = height;
    }
}

void Downsample::setInput(int index, Texture *source) {
    m_outputs.front().second = source;

    m_downPasses[0].downMaterial->setTexture(rgbMap, source);
}
