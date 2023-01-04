#include "pipelinepasses/guilayer.h"

#include "components/gui/widget.h"

#include "pipelinecontext.h"
#include "commandbuffer.h"

#include <cmath>
#include <cstring>

GuiLayer::GuiLayer() :
        m_width(0),
        m_height(0),
        m_uiAsSceneView(false) {

    setEnabled(true);
}

Texture *GuiLayer::draw(Texture *source, PipelineContext *context) {
    CommandBuffer *buffer = context->buffer();
    if(!m_uiAsSceneView) {
        buffer->setScreenProjection(0, 0, m_width, m_height);
    } else {
        context->cameraReset();
    }
    for(auto it : context->uiComponents()) {
        it->draw(*buffer, CommandBuffer::UI);
    }

    return source;
}

uint32_t GuiLayer::layer() const {
    return CommandBuffer::UI;
}

void GuiLayer::resize(int32_t width, int32_t height) {
    m_width = width;
    m_height = height;
}

void GuiLayer::showUiAsSceneView() {
    m_uiAsSceneView = true;
}
