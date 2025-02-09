#include "pipelinetasks/indirect.h"

#include "components/private/postprocessorsettings.h"

#include "components/world.h"
#include "components/camera.h"

#include "resources/material.h"
#include "resources/rendertarget.h"

#include "pipelinecontext.h"
#include "commandbuffer.h"

namespace {
    const char *environmentMap("deferredIndirect/environmentMap");
    const char *iblMap("iblMap");
};

DeferredIndirect::DeferredIndirect() :
        m_iblMaterial(nullptr),
        m_cameraTexture(Engine::objectCreate<Texture>("cameraTexture")),
        m_iblTexture(nullptr),
        m_iblTarget(Engine::objectCreate<RenderTarget>()) {

    setName("Indirect");

    m_inputs.push_back("In");
    m_inputs.push_back("Normals");
    m_inputs.push_back("Params");
    m_inputs.push_back("Depth");
    m_inputs.push_back("SSAO");
    m_inputs.push_back("SSLR");

    m_outputs.push_back(std::make_pair("Result", nullptr));

    PostProcessSettings::registerSetting(environmentMap, Variant::fromValue(m_iblTexture));

    // Building camera background color texture cubemap
    m_cameraTexture->setFormat(Texture::RGBA8);
    m_cameraTexture->setFiltering(Texture::Bilinear);
    m_cameraTexture->resize(1, 1);
    m_cameraTexture->clear();

    Texture::Surface surface;
    surface.push_back({0, 0, 0, 255});

    for(int i = 0; i < 6; i++) {
        m_cameraTexture->addSurface(surface);
    }

    // Building material instance
    Material *mtl = Engine::loadResource<Material>(".embedded/IblReflections.shader");
    if(mtl) {
        m_iblMaterial = mtl->createInstance();
        m_iblMaterial->setTexture(iblMap, m_cameraTexture);
    }

}

void DeferredIndirect::analyze(World *world) {
    Camera *camera = Camera::current();
    if(camera) {
        Vector4 color(camera->color());
        if(color != m_cameraColor) {
            m_cameraColor = color;

            for(int i = 0; i < 6; i++) {
                Texture::Surface &surface = m_cameraTexture->surface(i);
                surface[0][0] = uint8_t(m_cameraColor.x * 255);
                surface[0][1] = uint8_t(m_cameraColor.y * 255);
                surface[0][2] = uint8_t(m_cameraColor.z * 255);
                surface[0][3] = uint8_t(m_cameraColor.w * 255);
            }
            m_cameraTexture->setDirty();
        }
    }


}

void DeferredIndirect::exec(PipelineContext &context) {
    CommandBuffer *buffer = context.buffer();

    for(auto it : context.culledPostEffectSettings()) {
        Texture *texture = it.first->readValue(environmentMap).value<Texture *>();
        if(texture != m_iblTexture) {
            m_iblTexture = texture;
            if(m_iblMaterial) {
                m_iblMaterial->setTexture(iblMap, m_iblTexture ? m_iblTexture : m_cameraTexture);
            }
        }
    }

    buffer->beginDebugMarker("ReflectionIndirect");
    if(m_iblMaterial) {
        buffer->setRenderTarget(m_iblTarget);
        buffer->drawMesh(PipelineContext::defaultPlane(), 0, CommandBuffer::UI, *m_iblMaterial);
    }
    buffer->endDebugMarker();
}

void DeferredIndirect::setInput(int index, Texture *texture) {
    if(m_iblMaterial) {
        switch(index) {
            case 0: { // In
                m_iblTarget->setColorAttachment(0, texture);
                m_outputs.front().second = texture;
            } break;
            case 1: { // normalsMap
                m_iblMaterial->setTexture("normalsMap", texture);
            } break;
            case 2: { // paramsMap
                m_iblMaterial->setTexture("paramsMap", texture);
            } break;
            case 3: { // depthMap
                m_iblMaterial->setTexture("depthMap", texture);
            } break;
            case 4: { // aoMap
                m_iblMaterial->setTexture("aoMap", texture);
            } break;
            case 5: { // sslrMap
                m_iblMaterial->setTexture("sslrMap", texture);
            } break;
            default: break;
        }
    }
}
