#include "createmodule.h"

#include "effectrootnode.h"
#include "effectmodule.h"

#include "../particleedit.h"

CreateModule::CreateModule(const std::string &module, EffectGraph *graph, ParticleEdit *editor, const QString &name, QUndoCommand *group) :
        UndoCommand(name, editor, group),
        m_moduleName(module),
        m_graph(graph),
        m_object(0) {

}

void CreateModule::undo() {
    EffectModule *module = dynamic_cast<EffectModule *>(Engine::findObject(m_object));
    if(module) {
        EffectRootNode *root = static_cast<EffectRootNode *>(m_graph->defaultNode());
        root->removeModule(module);

        m_graph->emitSignal(_SIGNAL(moduleChanged()));
    }
}

void CreateModule::redo() {
    EffectRootNode *root = static_cast<EffectRootNode *>(m_graph->defaultNode());
    EffectModule *module = root->insertModule(m_graph->modulePath(m_moduleName));
    if(module) {
        m_object = module->uuid();

        m_graph->emitSignal(_SIGNAL(moduleChanged()));
    }
}
