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

#include "pipelinetasks/translucent.h"

#include "resources/rendertarget.h"

#include "components/renderable.h"

#include "commandbuffer.h"

Translucent::Translucent() :
        m_translucentPass(Engine::objectCreate<RenderTarget>("translucentPass")) {

    setName("Translucent");

    m_inputs.push_back("In");
    m_inputs.push_back("Depth");

    m_outputs.push_back(std::make_pair("Result", nullptr));
}

void Translucent::exec() {
    CommandBuffer *buffer = m_context->buffer();

    if(!m_translucent.empty()) {
        buffer->beginDebugMarker("TranslucentPass");

        buffer->setRenderTarget(m_translucentPass);

        for(auto &it : m_translucent) {
            if(it.count > 1) {
                it.instance->setInstanceBuffer(&it.buffer);
            }
            buffer->drawMesh(it.mesh, it.subMesh, Material::Translucent, *it.instance);
            it.instance->setInstanceBuffer(nullptr);
        }

        buffer->endDebugMarker();
    }
}

void Translucent::setInput(int index, Texture *texture) {
    switch(index) {
        case 0: {
            m_translucentPass->setColorAttachment(0, texture);
            m_outputs.front().second = texture;
        } break;
        case 1: {
            m_translucentPass->setDepthAttachment(texture);
        } break;
        default: break;
    }
}

void Translucent::analyze(World *world) {
    A_UNUSED(world);
    Renderable::GroupList list;
    Renderable::filterByLayer(m_context->culledRenderables(), list, Material::Translucent);

    m_translucent.clear();
    Renderable::group(list, m_translucent);
}
