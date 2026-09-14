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
#include "components/canvas.h"

#include <commandbuffer.h>
#include <input.h>

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

    m_canvas.clear();
    static uint32_t canvasHash = Mathf::hashString("canvas");
    for(auto scene : world->scenes()) {
        for(auto it : scene->getObjectsInGroupByHash(canvasHash)) {
            Canvas *canvas = static_cast<Canvas *>(it);
            if(canvas->isEnabledInHierarchy()) {
                canvas->setSize(m_width, m_height);
                canvas->update(pos);
                m_canvas.push_back(canvas);
            }
        }
    }
}

void GuiLayer::exec() {
    CommandBuffer *buffer = m_context->buffer();

    buffer->beginDebugMarker("GuiLayer");

    for(auto it : m_canvas) {
        it->draw(buffer);
    }

    buffer->endDebugMarker();
}

void GuiLayer::setInput(int index, Texture *source) {
    m_outputs.front().second = source;
}
