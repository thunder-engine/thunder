#include "deletemodule.h"

#include "effectmodule.h"
#include "effectrootnode.h"

DeleteModule::DeleteModule(EffectModule *module, EffectGraph *graph, const QString &name, QUndoCommand *group) :
        UndoCommand(name, graph, group),
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
            module->fromXml(m_document.firstChild().toElement());

            m_graph->moduleChanged();
        }
    }
}

void DeleteModule::redo() {
    m_document.clear();

    EffectModule *module = static_cast<EffectModule *>(Engine::findObject(m_object));
    if(module) {
        m_path = module->path();

        QDomElement moduleElement = m_document.createElement("module");
        module->toXml(moduleElement, m_document);

        m_document.appendChild(moduleElement);

        EffectRootNode *root = static_cast<EffectRootNode *>(module->parent());
        m_root = root->uuid();
        m_index = root->moduleIndex(module);

        root->removeModule(module);

        m_graph->moduleChanged();
    }
}
