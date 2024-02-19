#include "pipelinetasks/guilayer.h"

#include "pipelinecontext.h"
#include "commandbuffer.h"
#include "uisystem.h"

#include <cmath>
#include <cstring>

#include "components/actor.h"
#include "components/world.h"
#include "components/widget.h"

GuiLayer::GuiLayer() :
        m_uiAsSceneView(false) {

    setEnabled(true);

    m_inputs.push_back("In");
    m_outputs.push_back(make_pair("Result", nullptr));
}

void GuiLayer::analyze(World *world) {
    m_uiComponents.clear();

    bool update = world->isToBeUpdated();

    for(auto it : UiSystem::widgets()) {
        if(it->isEnabled()) {
            Actor *actor = it->actor();
            if(actor && actor->isEnabledInHierarchy()) {
                if((actor->world() == world)) {
                    if(update) {
                        static_cast<NativeBehaviour *>(it)->update();
                    }
                    m_uiComponents.push_back(it);
                }
            }
        }
    }
}

void GuiLayer::exec(PipelineContext &context) {
    CommandBuffer *buffer = context.buffer();

    buffer->beginDebugMarker("GuiLayer");

    if(!m_uiAsSceneView) {
        buffer->setScreenProjection(0, 0, m_width, m_height);
    } else {
        context.cameraReset();
    }

    for(auto it : m_uiComponents) {
        it->draw(*buffer);
    }

    buffer->endDebugMarker();
}

void GuiLayer::showUiAsSceneView(bool flag) {
    m_uiAsSceneView = flag;
}

void GuiLayer::setInput(int index, Texture *source) {
    m_outputs.front().second = source;
}
