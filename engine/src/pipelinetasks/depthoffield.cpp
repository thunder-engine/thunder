#include "pipelinetasks/depthoffield.h"

#include "components/private/postprocessorsettings.h"

#include "engine.h"

#include "pipelinecontext.h"
#include "commandbuffer.h"

#include "resources/material.h"
#include "resources/rendertarget.h"

#include "components/camera.h"

namespace {
    const char *gDepthOfField("graphics.depthOfField");

    const char *gFocusDistance("focusDistance");
    const char *gFocusScale("focusScale");
    const char *gBlurSize("blurSize");
    const char *gSkyDistance("skyDistance");

    const char *dofFocusScale("depthOfField/focusScale");
    const char *dofBlurSize("depthOfField/blurSize");
    const char *dofSkyDistance("depthOfField/skyDistance");
};

DepthOfField::DepthOfField() :
        m_resultTexture(Engine::objectCreate<Texture>("depthOfField")),
        m_resultTarget(Engine::objectCreate<RenderTarget>("depthOfField")),
        m_dofMaterial(nullptr),
        m_focusDistance(1.0f),
        m_focusScale(10.0f),
        m_blurSize(20.0f),
        m_skyDistance(100000.0f) {

    m_enabled = false;
    setName("DepthOfField");

    m_inputs.push_back("In");
    m_inputs.push_back("Downsample");
    m_inputs.push_back("Depth");

    Engine::setValue(gDepthOfField, false);

    PostProcessSettings::registerSetting(dofFocusScale, m_focusScale);
    PostProcessSettings::registerSetting(dofBlurSize, m_blurSize);
    PostProcessSettings::registerSetting(dofSkyDistance, m_skyDistance);

    Material *material = Engine::loadResource<Material>(".embedded/DOF.shader");
    if(material) {
        m_dofMaterial = material->createInstance();
        m_dofMaterial->setFloat(gFocusDistance, &m_focusDistance);

        float scale = 1.0f / m_focusScale;
        m_dofMaterial->setFloat(gFocusScale, &scale);
        m_dofMaterial->setFloat(gBlurSize, &m_blurSize);
        m_dofMaterial->setFloat(gSkyDistance, &m_skyDistance);
    }

    m_resultTexture->setFormat(Texture::RGB10A2);
    m_resultTexture->setFiltering(Texture::Bilinear);
    m_resultTexture->setFlags(Texture::Render);

    m_resultTarget->setColorAttachment(0, m_resultTexture);

    m_outputs.push_back(std::make_pair(m_resultTexture->name(), m_resultTexture));
}

DepthOfField::~DepthOfField() {
    m_resultTarget->deleteLater();
    m_resultTexture->deleteLater();

    delete m_dofMaterial;
}

void DepthOfField::exec() {
    if(m_dofMaterial) {
        CommandBuffer *buffer = m_context->buffer();

        // Focus Distance
        Camera *camera = Camera::current();
        float focal  = camera->focal();
        if(focal != m_focusDistance) {
            m_focusDistance = focal;
            if(m_dofMaterial) {
                m_dofMaterial->setFloat(gFocusDistance, &m_focusDistance);
            }
        }

        // Focus Scale
        float focusScale = PostProcessSettings::defaultValue(gFocusScale).toFloat();
        for(auto pool : m_context->culledPostEffectSettings()) {
            const PostProcessSettings *settings = pool.first;
            Variant value = settings->readValue(gFocusScale);
            if(value.isValid()) {
                focusScale = MIX(m_focusScale, value.toFloat(), pool.second);
            }
        }
        if(focusScale != m_focusScale) {
            m_focusScale = focusScale;
            if(m_dofMaterial) {
                float scale = 1.0f / m_focusScale;
                m_dofMaterial->setFloat(gFocusScale, &scale);
            }
        }

        // Blur Size
        float blurSize = PostProcessSettings::defaultValue(gBlurSize).toFloat();
        for(auto pool : m_context->culledPostEffectSettings()) {
            const PostProcessSettings *settings = pool.first;
            Variant value = settings->readValue(gBlurSize);
            if(value.isValid()) {
                blurSize = MIX(m_blurSize, value.toFloat(), pool.second);
            }
        }
        if(blurSize != m_blurSize) {
            m_blurSize = blurSize;
            if(m_dofMaterial) {
                m_dofMaterial->setFloat(gBlurSize, &m_blurSize);
            }
        }

        // Sky Distance
        float skyDistance = PostProcessSettings::defaultValue(gSkyDistance).toFloat();
        for(auto pool : m_context->culledPostEffectSettings()) {
            const PostProcessSettings *settings = pool.first;
            Variant value = settings->readValue(gSkyDistance);
            if(value.isValid()) {
                skyDistance = MIX(m_skyDistance, value.toFloat(), pool.second);
            }
        }
        if(skyDistance != m_skyDistance) {
            m_skyDistance = skyDistance;
            if(m_dofMaterial) {
                m_dofMaterial->setFloat(gSkyDistance, &m_skyDistance);
            }
        }

        buffer->beginDebugMarker("DepthOfField");

        buffer->setRenderTarget(m_resultTarget);
        buffer->drawMesh(PipelineContext::defaultPlane(), 0, CommandBuffer::UI, *m_dofMaterial);

        buffer->endDebugMarker();
    }
}

void DepthOfField::setInput(int index, Texture *texture) {
    if(m_enabled) {
        if(m_dofMaterial) {
            switch(index) {
                case 0: m_dofMaterial->setTexture("highMap", texture); break;
                case 1: m_dofMaterial->setTexture("lowMap", texture); break;
                case 2: m_dofMaterial->setTexture("depthMap", texture); break;
                default: break;
            }
        }
        m_resultTexture->resize(m_width, m_height);

        m_outputs.back().second = m_resultTexture;
    } else if(index == 0) {
        m_outputs.back().second = texture;
    }
}
