#include "deletemodule.h"

#include "effectmodule.h"
#include "effectrootnode.h"

#include "../particleedit.h"

DeleteModule::DeleteModule(EffectModule *module, EffectGraph *graph, const TString &name, UndoCommand *group) :
        UndoCommand(name, group),
        m_graph(graph),
        m_object(module->uuid()),
        m_root(0),
        m_index(-1) {

}

void DeleteModule::undo() {
    EffectRootNode *root = static_cast<EffectRootNode *>(Engine::findObject(m_root));
    if(root) {
        EffectModule *module = root->insertModule(m_path, m_index);
        if(module) {
            module->fromXml(m_document.first_child());

            m_graph->emitSignal(_SIGNAL(moduleChanged()));
        }
    }
}

void DeleteModule::redo() {
    m_document.reset();

    EffectModule *module = static_cast<EffectModule *>(Engine::findObject(m_object));
    if(module) {
        m_path = module->path();

        pugi::xml_node moduleElement = m_document.append_child("module");
        module->toXml(moduleElement);

        EffectRootNode *root = static_cast<EffectRootNode *>(module->parent());
        m_root = root->uuid();
        m_index = root->moduleIndex(module);

        root->removeModule(module);

        m_graph->emitSignal(_SIGNAL(moduleChanged()));
    }
}
