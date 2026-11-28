/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "pipelinetasks/antialiasing.h"

#include "engine.h"

#include "pipelinecontext.h"
#include "commandbuffer.h"

#include "resources/material.h"
#include "resources/rendertarget.h"

namespace {
    const char *gAntialiasing("g.antialiasing");
};

AntiAliasing::AntiAliasing() :
        m_resultTexture(Engine::objectCreate<Texture>("antiAliasing")),
        m_resultTarget(Engine::objectCreate<RenderTarget>("antiAliasing")),
        m_resultMaterial(nullptr) {

    setName("AntiAliasing");

    m_inputs.push_back("In");

    Engine::setValue(gAntialiasing, 1);

    Material *material = Engine::loadResource<Material>(".engine/FXAA.shader");
    if(material) {
        m_resultMaterial = material->createInstance();
    }

    m_resultTexture->setFormat(Texture::RGBA16Float);
    m_resultTexture->setFlags(Texture::Render);

    m_resultTarget->setColorAttachment(0, m_resultTexture);

    m_outputs.push_back(std::make_pair(m_resultTexture->name(), m_resultTexture));
}

AntiAliasing::~AntiAliasing() {
    m_resultTarget->deleteLater();
}

void AntiAliasing::exec() {
    if(m_resultMaterial) {
        CommandBuffer *buffer = m_context->buffer();
        buffer->beginDebugMarker("AntiAliasing");

        buffer->setRenderTarget(m_resultTarget);
        buffer->drawMesh(PipelineContext::defaultPlane(), 0, Material::Opaque, *m_resultMaterial);

        buffer->endDebugMarker();
    }
}

void AntiAliasing::setInput(int index, Texture *texture) {
    A_UNUSED(index);
    if(m_enabled) {
        if(m_resultMaterial) {
            m_resultMaterial->setTexture("rgbMap", texture);
        }
        m_outputs.back().second = m_resultTexture;
    } else {
        m_outputs.back().second = texture;
    }
}
