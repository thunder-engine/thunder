#include "pipelinetasks/depthoffield.h"

#include "components/private/postprocessorsettings.h"

#include "engine.h"

#include "pipelinecontext.h"
#include "commandbuffer.h"

#include "resources/material.h"
#include "resources/rendertarget.h"

#include "components/camera.h"

#include <cstring>

#define MAX_SAMPLES 32

namespace {
    const char *gDepthOfField("g.depthOfField");

    const char *gFocusDistance("focusDistance");
    const char *gFocusScale("focusScale");
    const char *gBlurSize("blurSize");
    const char *gSkyDistance("skyDistance");

    const char *gIntensity("intensity");
    const char *gDirtIntensity("dirtIntensity");

    const char *gDirection("direction");
    const char *gSteps("steps");
    const char *gCurve("curve");
    const char *gSize("size");

    const char *gRgbMap("rgbMap");

    const char *dofFocusScale("depthOfField/focusScale");
    const char *dofBlurSize("depthOfField/blurSize");
    const char *dofSkyDistance("depthOfField/skyDistance");
};

DepthOfField::DepthOfField() :
        m_resultTexture(Engine::objectCreate<Texture>("depthOfField")),
        m_downTexture(Engine::objectCreate<Texture>("downTexture")),
        m_blurTexture(Engine::objectCreate<Texture>("downTexture")),
        m_resultTarget(Engine::objectCreate<RenderTarget>("depthOfField")),
        m_downTarget(Engine::objectCreate<RenderTarget>("downTarget")),
        m_blurTarget(Engine::objectCreate<RenderTarget>("blurTarget")),
        m_focusDistance(1.0f),
        m_focusScale(10.0f),
        m_blurSize(20.0f),
        m_skyDistance(100000.0f) {

    m_enabled = false;
    setName("DepthOfField");

    m_inputs.push_back("In");
    m_inputs.push_back("Depth");

    Engine::setValue(gDepthOfField, m_enabled);

    PostProcessSettings::registerSetting(dofFocusScale, m_focusScale);
    PostProcessSettings::registerSetting(dofBlurSize, m_blurSize);
    PostProcessSettings::registerSetting(dofSkyDistance, m_skyDistance);

    Material *downSample = Engine::loadResource<Material>(".embedded/Downsample.shader");
    if(downSample) {
        m_downMaterial = downSample->createInstance();
    }

    Material *blur = Engine::loadResource<Material>(".embedded/Blur.shader");
    if(blur) {
        float intensity = 1.0f;
        float dirtIntensity = 0.0f;
        m_blurMaterial = blur->createInstance();
        m_blurMaterial->setFloat(gIntensity, &intensity);
        m_blurMaterial->setFloat(gDirtIntensity, &dirtIntensity);
    }

    Material *dof = Engine::loadResource<Material>(".embedded/DOF.shader");
    if(dof) {
        m_dofMaterial = dof->createInstance();
        m_dofMaterial->setFloat(gFocusDistance, &m_focusDistance);

        float scale = 1.0f / m_focusScale;
        m_dofMaterial->setFloat(gFocusScale, &scale);
        m_dofMaterial->setFloat(gBlurSize, &m_blurSize);
        m_dofMaterial->setFloat(gSkyDistance, &m_skyDistance);
        m_dofMaterial->setTexture("lowMap", m_downTexture);
    }

    m_resultTexture->setFormat(Texture::RGBA16Float);
    m_resultTexture->setFiltering(Texture::Bilinear);
    m_resultTexture->setFlags(Texture::Render);

    m_downTexture->setFormat(Texture::RGBA16Float);
    m_downTexture->setFiltering(Texture::Bilinear);
    m_downTexture->setFlags(Texture::Render);

    m_blurTexture->setFormat(Texture::RGBA16Float);
    m_blurTexture->setFiltering(Texture::Bilinear);
    m_blurTexture->setFlags(Texture::Render);

    m_resultTarget->setColorAttachment(0, m_resultTexture);

    m_downTarget->setColorAttachment(0, m_downTexture);
    m_downTarget->setClearFlags(RenderTarget::ClearColor);

    m_blurTarget->setColorAttachment(0, m_blurTexture);
    m_blurTarget->setClearFlags(RenderTarget::ClearColor);

    m_outputs.push_back(std::make_pair(m_resultTexture->name(), m_resultTexture));
}

DepthOfField::~DepthOfField() {
    m_resultTarget->deleteLater();
    m_downTarget->deleteLater();

    m_resultTexture->deleteLater();
    m_downTexture->deleteLater();

    delete m_dofMaterial;
    delete m_downMaterial;
}

void DepthOfField::analyze(World *world) {
    // Focus Distance
    Camera *camera = Camera::current();
    float focal  = camera->focalDistance();
    if(focal != m_focusDistance) {
        m_focusDistance = focal;
        if(m_dofMaterial) {
            m_dofMaterial->setFloat(gFocusDistance, &m_focusDistance);
        }
    }

    // Focus Scale
    float focusScale = PostProcessSettings::defaultValue(dofFocusScale).toFloat();
    for(auto pool : m_context->culledPostEffectSettings()) {
        const PostProcessSettings *settings = pool.first;
        Variant value = settings->readValue(dofFocusScale);
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
    float blurSize = PostProcessSettings::defaultValue(dofBlurSize).toFloat();
    for(auto pool : m_context->culledPostEffectSettings()) {
        const PostProcessSettings *settings = pool.first;
        Variant value = settings->readValue(dofBlurSize);
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
    float skyDistance = PostProcessSettings::defaultValue(dofSkyDistance).toFloat();
    for(auto pool : m_context->culledPostEffectSettings()) {
        const PostProcessSettings *settings = pool.first;
        Variant value = settings->readValue(dofSkyDistance);
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
}

void DepthOfField::exec() {
    if(m_dofMaterial) {
        CommandBuffer *buffer = m_context->buffer();

        buffer->beginDebugMarker("DepthOfField");

        buffer->setViewport(0, 0, m_downTexture->width(), m_downTexture->height());
        buffer->setRenderTarget(m_downTarget);
        buffer->drawMesh(PipelineContext::defaultPlane(), 0, Material::Opaque, *m_downMaterial);

        Vector2 direction;

        buffer->setRenderTarget(m_blurTarget);
        direction.x = 1.0f;
        direction.y = 0.0f;
        m_blurMaterial->setVector2(gDirection, &direction);
        m_blurMaterial->setTexture(gRgbMap, m_downTexture);
        buffer->drawMesh(PipelineContext::defaultPlane(), 0, Material::Opaque, *m_blurMaterial);

        buffer->setRenderTarget(m_downTarget);
        direction.x = 0.0f;
        direction.y = 1.0f;
        m_blurMaterial->setVector2(gDirection, &direction);
        m_blurMaterial->setTexture(gRgbMap, m_blurTexture);
        buffer->drawMesh(PipelineContext::defaultPlane(), 0, Material::Opaque, *m_blurMaterial);

        buffer->setRenderTarget(m_resultTarget);
        buffer->setViewport(0, 0, m_resultTexture->width(), m_resultTexture->height());
        buffer->drawMesh(PipelineContext::defaultPlane(), 0, Material::Opaque, *m_dofMaterial);

        buffer->endDebugMarker();
    }
}

void DepthOfField::resize(int32_t width, int32_t height) {
    if(m_width != width || m_height != height) {

        m_downTexture->resize((width >> 1), (height >> 1));
        m_blurTexture->resize((width >> 1), (height >> 1));
        m_resultTexture->resize(width, height);

        float radius = MAX((width >> 1), 1) * 4.0f * 0.01f;
        int steps = CLAMP(static_cast<int32_t>(radius), 0, MAX_SAMPLES);

        Vector2 size(1.0f / (float)(width >> 1), 1.0f / (float)(height >> 1));

        float blurPoints[MAX_SAMPLES];
        generateKernel(radius, steps, blurPoints);

        m_blurMaterial->setInteger(gSteps, &steps);
        m_blurMaterial->setFloat(gCurve, blurPoints);
        m_blurMaterial->setVector2(gSize, &size);
    }

    PipelineTask::resize(width, height);
}

void DepthOfField::setInput(int index, Texture *texture) {
    if(m_enabled) {
        if(m_dofMaterial) {
            switch(index) {
                case 0: {
                    m_downMaterial->setTexture("rgbMap", texture);
                    m_dofMaterial->setTexture("highMap", texture);
                } break;
                case 1: m_dofMaterial->setTexture("depthMap", texture); break;
                default: break;
            }
        }

        m_outputs.back().second = m_resultTexture;
    } else if(index == 0) {
        m_outputs.back().second = texture;
    }
}

void DepthOfField::generateKernel(float radius, int32_t steps, float *points) {
    memset(points, 0, sizeof(float) * MAX_SAMPLES);

    float total = 0.0f;
    for(uint8_t p = 0; p < steps; p++) {
        float weight = std::exp(-static_cast<float>(p * p) / (2.0f * radius));
        points[p] = weight;

        total += weight;
    }

    for(uint8_t p = 0; p < steps; p++) {
        points[p] *= 1.0f / total * 0.5f; // 1.0 / (sqrt(2.0 * PI) * sigma;
    }
}
