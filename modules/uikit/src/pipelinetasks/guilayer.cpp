#include "pipelinetasks/guilayer.h"

#include "pipelinecontext.h"
#include "commandbuffer.h"
#include "uisystem.h"

#include <cmath>
#include <cstring>

#include "components/actor.h"
#include "components/world.h"
#include "components/widget.h"
#include "components/recttransform.h"

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
        buffer->setViewProjection(Matrix4(), Matrix4::ortho(0, m_width, 0, m_height, 0.0f, 100.0f));
    } else {
        context.cameraReset();
    }

    for(auto it : m_uiComponents) {
        if(!m_uiAsSceneView && it->parentWidget() == nullptr && it->rectTransform()) {
            it->rectTransform()->setSize(buffer->viewport());
        }

        it->draw(*buffer);
    }

    buffer->endDebugMarker();
}

void GuiLayer::setInput(int index, Texture *source) {
    m_outputs.front().second = source;
}

void GuiLayer::setProperty(const string &name, const Variant &value) {
    if(name == "sceneView") {
        m_uiAsSceneView = value.toBool();
    }
}
