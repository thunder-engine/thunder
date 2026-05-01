#include "pipelinetasks/guilayer.h"

#include "uisystem.h"

#include "components/world.h"
#include "components/widget.h"
#include "components/recttransform.h"

#include <pipelinecontext.h>
#include <commandbuffer.h>
#include <input.h>

GuiLayer::GuiLayer() {

    setName("GuiLayer");

    m_inputs.push_back("In");
    m_outputs.push_back(std::make_pair("Result", nullptr));
}

void GuiLayer::analyze(World *world) {
    m_uiComponents.clear();

    Vector4 pos = Input::mousePosition();
    if(Input::touchCount() > 0) {
        pos = Input::touchPosition(0);
    }

    for(auto it : UiSystem::widgets()) {
        if(it->isEnabled() && it->world() == world && it->parentWidget() == nullptr) {
            it->update(pos);
            m_uiComponents.push_back(it);
        }
    }
}

void GuiLayer::exec() {
    CommandBuffer *buffer = m_context->buffer();

    buffer->beginDebugMarker("GuiLayer");

    Matrix4 v;
    v[14] = -50.0f;
    buffer->setViewProjection(v, Matrix4::ortho(0, m_width, 0, m_height, 0.0f, 100.0f));

    for(auto it : m_uiComponents) {
        if(it->parentWidget() == nullptr) { // Root widget
            RectTransform *rect = it->rectTransform();
            if(rect) {
                rect->setSize(m_context->size());
            }

            it->draw(*buffer);

            break;
        }
    }

    buffer->endDebugMarker();
}

void GuiLayer::setInput(int index, Texture *source) {
    m_outputs.front().second = source;
}
