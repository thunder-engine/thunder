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
