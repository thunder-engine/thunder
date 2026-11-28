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

#include "pipelinetasks/gbuffer.h"

#include "resources/rendertarget.h"

#include "pipelinecontext.h"
#include "commandbuffer.h"

#define DEPTH_MAP   "depthMap"
#define G_EMISSIVE  "emissiveMap"
#define G_NORMALS   "normalsMap"
#define G_DIFFUSE   "diffuseMap"
#define G_PARAMS    "paramsMap"

GBuffer::GBuffer() :
        m_gbuffer(Engine::objectCreate<RenderTarget>("gBuffer")) {

    setName("GBuffer");

    Texture *emissive = Engine::objectCreate<Texture>(G_EMISSIVE);
    emissive->setFormat(Texture::RGBA16Float);
    emissive->setFlags(Texture::Render);
    m_outputs.push_back(std::make_pair(emissive->name(), emissive));

    Texture *normals = Engine::objectCreate<Texture>(G_NORMALS);
    normals->setFormat(Texture::RGB10A2);
    normals->setFlags(Texture::Render);
    m_outputs.push_back(std::make_pair(normals->name(), normals));

    Texture *diffuse = Engine::objectCreate<Texture>(G_DIFFUSE);
    diffuse->setFormat(Texture::RGBA8);
    diffuse->setFlags(Texture::Render);
    m_outputs.push_back(std::make_pair(diffuse->name(), diffuse));

    Texture *params = Engine::objectCreate<Texture>(G_PARAMS);
    params->setFormat(Texture::RGBA8);
    params->setFlags(Texture::Render);
    m_outputs.push_back(std::make_pair(params->name(), params));

    Texture *depth = Engine::objectCreate<Texture>(DEPTH_MAP);
    depth->setFormat(Texture::Depth);
    depth->setDepthBits(24);
    depth->setFlags(Texture::Render);

    m_outputs.push_back(std::make_pair(depth->name(), depth));

    for(int i = 0; i < m_outputs.size(); i++) {
        if(m_outputs[i].second->depthBits() > 0) {
            m_gbuffer->setDepthAttachment(m_outputs[i].second);
        } else {
            m_gbuffer->setColorAttachment(i, m_outputs[i].second);
        }
    }

    m_gbuffer->setFlags(RenderTarget::ClearColor | RenderTarget::ClearDepth);
}

void GBuffer::exec() {
    CommandBuffer *buffer = m_context->buffer();
    buffer->beginDebugMarker("GBuffer Pass");

    buffer->setViewport(0, 0, m_width, m_height);
    m_context->cameraReset();

    buffer->setRenderTarget(m_gbuffer);

    for(auto &it : m_opaque) {
        if(it.count > 1) {
            it.instance->setInstanceBuffer(&it.buffer);
        }
        buffer->drawMesh(it.mesh, it.subMesh, Material::Opaque, *it.instance);
        it.instance->setInstanceBuffer(nullptr);
    }

    buffer->endDebugMarker();
}

void GBuffer::analyze(World *world) {
    A_UNUSED(world);
    Renderable::GroupList list;
    Renderable::filterByLayer(m_context->culledRenderables(), list, Material::Opaque);

    m_opaque.clear();
    Renderable::group(list, m_opaque);
}
