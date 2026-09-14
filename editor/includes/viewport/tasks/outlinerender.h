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
#ifndef OUTLINERENDER_H
#define OUTLINERENDER_H

#include "pipelinetask.h"
#include "pipelinecontext.h"
#include "commandbuffer.h"

#include "rendertarget.h"

#include "renderable.h"

#include "editor/editorsettings.h"

#include "../cameracontroller.h"

namespace {
    const char *gOutlineWidth("Viewport/Outline/Width");
    const char *gOutlineColor("Viewport/Outline/Color");
}

class Outline : public PipelineTask {
public:
    Outline() :
            m_width(1.0f),
            m_outlineMap(Engine::objectCreate<Texture>()),
            m_outlineDepth(Engine::objectCreate<Texture>()),
            m_outlineTarget(Engine::objectCreate<RenderTarget>("Outline.Target")),
            m_resultTarget(Engine::objectCreate<RenderTarget>("Outline.ResultTarget")),
            m_controller(nullptr) {

        m_outlineDepth->setFormat(Texture::Depth);
        m_outlineDepth->setDepthBits(24);
        m_outlineDepth->setFlags(Texture::Render);

        m_outlineMap->setFormat(Texture::RGBA8);
        m_outlineMap->setFlags(Texture::Render);

        m_outlineTarget->setColorAttachment(0, m_outlineMap);
        m_outlineTarget->setDepthAttachment(m_outlineDepth);
        m_outlineTarget->setFlags(RenderTarget::ClearColor | RenderTarget::ClearDepth);

        Material *material = Engine::loadResource<Material>(".embedded/outline.shader");
        if(material) {
            m_combineMaterial = material->createInstance();
            m_combineMaterial->setTexture("outlineMap", m_outlineMap);
        }

        setName("Outline");

        m_inputs.push_back("In");

        m_outputs.push_back(std::make_pair("Result", nullptr));

        connect(Editor::settings(), _SIGNAL(updated()), this, _SLOT(loadSettings()));

        Editor::settings()->registerValue(gOutlineColor, Vector4(1.0f, 0.5f, 0.0f, 1.0f), "editor=Color");
        Editor::settings()->registerValue(gOutlineWidth, 1.0f);

        loadSettings();
    }

    void loadSettings() {
        m_color = Editor::settings()->value(gOutlineColor).toVector4();
        m_width = Editor::settings()->value(gOutlineWidth).toFloat();

        if(m_combineMaterial) {
            m_combineMaterial->setFloat("width", &m_width);
            m_combineMaterial->setVector4("color", &m_color);
        }
    }

    void setController(CameraController *controller) {
        m_controller = controller;
    }

    void setInput(int index, Texture *texture) override {
        if(texture) {
            m_resultTarget->setColorAttachment(index, texture);

            m_outputs.front().second = texture;
        }
    }

private:
    void exec() override {
        if(m_combineMaterial && m_controller) {
            CommandBuffer *buffer = m_context->buffer();
            buffer->beginDebugMarker("Outline");

            buffer->setRenderTarget(m_outlineTarget);

            for(auto &it : m_outline) {
                buffer->drawMesh(it.mesh, it.subMesh, Material::Visibility, *it.instance);
            }

            buffer->setRenderTarget(m_resultTarget);
            buffer->drawMesh(PipelineContext::defaultPlane(), 0, Material::Opaque, *m_combineMaterial);

            buffer->endDebugMarker();
        }
    }

    void resize(int32_t width, int32_t height) override {
        m_outlineMap->resize(width, height);
        m_outlineDepth->resize(width, height);

        PipelineTask::resize(width, height);
    }

    void analyze(World *world) override {
        RenderList outline;

        for(auto actor : m_controller->selected()) {
            for(auto it : m_context->culledRenderables()) {
                if(it->actor()->isInHierarchy(static_cast<Actor *>(actor))) {
                    outline.push_back(it);
                }
            }
        }

        m_outline.clear();
        Renderable::filterByLayer(outline, m_outline, Material::Visibility);
    }

protected:
    Vector4 m_color;

    Renderable::GroupList m_outline;

    Texture *m_outlineMap;
    Texture *m_outlineDepth;

    RenderTarget *m_outlineTarget;
    RenderTarget *m_resultTarget;

    MaterialInstance *m_combineMaterial;

    CameraController *m_controller;

    float m_width;

};

#endif // OUTLINERENDER_H
