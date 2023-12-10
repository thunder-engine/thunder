#include "pipelinetasks/guilayer.h"

#include "components/gui/widget.h"

#include "pipelinecontext.h"
#include "commandbuffer.h"

#include <cmath>
#include <cstring>

GuiLayer::GuiLayer() :
        m_uiAsSceneView(false) {

    setEnabled(true);

    m_inputs.push_back("In");
    m_outputs.push_back(make_pair("Result", nullptr));
}

void GuiLayer::exec(PipelineContext *context) {
    CommandBuffer *buffer = context->buffer();

    buffer->beginDebugMarker("GuiLayer");

    if(!m_uiAsSceneView) {
        buffer->setScreenProjection(0, 0, m_width, m_height);
    } else {
        context->cameraReset();
    }

    for(auto it : context->uiComponents()) {
        it->draw(*buffer, CommandBuffer::UI);
    }

    buffer->endDebugMarker();
}

void GuiLayer::showUiAsSceneView(bool flag) {
    m_uiAsSceneView = flag;
}

void GuiLayer::setInput(int index, Texture *source) {
    m_outputs.front().second = source;
}
