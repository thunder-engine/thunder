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
#include "pipelinetasks/guilayer.h"

#include "components/world.h"
#include "components/scene.h"
#include "components/actor.h"
#include "components/canvas.h"

#include <commandbuffer.h>
#include <input.h>

void GuiLayer::collectCanvases(Object *object, std::stack<Canvas *> &canvases, int width, int height, const Vector4 &position) {
    Object::ObjectList children = object->getChildren();

    Actor *actor = dynamic_cast<Actor *>(object);
    if(actor) {
        Canvas *canvas = actor->getComponent<Canvas>();
        if(canvas && canvas->isEnabledInHierarchy()) {
            canvas->setSize(width, height);
            canvas->update(position);
            canvases.push(canvas);
        }
    }

    for(auto child : children) {
        collectCanvases(child, canvases, width, height, position);
    }
}

GuiLayer::GuiLayer() {

    setName("GuiLayer");

    m_inputs.push_back("In");
    m_outputs.push_back(std::make_pair("Result", nullptr));
}

void GuiLayer::analyze(World *world) {
    Vector4 pos = Input::mousePosition();
    if(Input::touchCount() > 0) {
        pos = Input::touchPosition(0);
    }

    while(!m_canvas.empty()) {
        m_canvas.pop();
    }

    for(auto scene : world->scenes()) {
        for(auto object : scene->getChildren()) {
            collectCanvases(object, m_canvas, m_width, m_height, pos);
        }
    }
}

void GuiLayer::exec() {
    CommandBuffer *buffer = m_context->buffer();

    buffer->beginDebugMarker("GuiLayer");

    while(!m_canvas.empty()) {
        m_canvas.top()->draw(buffer);
        m_canvas.pop();
    }

    buffer->endDebugMarker();
}

void GuiLayer::setInput(int index, Texture *source) {
    m_outputs.front().second = source;
}
