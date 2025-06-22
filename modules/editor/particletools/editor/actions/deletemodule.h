#ifndef DELETEMODULE_H
#define DELETEMODULE_H

#include "effectgraph.h"

class EffectModule;

class DeleteModule : public UndoCommand {
public:
    DeleteModule(EffectModule *module, EffectGraph *graph, const QString &name, QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    std::string m_path;

    QDomDocument m_document;

    EffectGraph *m_graph;

    uint32_t m_object;
    uint32_t m_root;
    int32_t m_index;

};

#endif // DELETEMODULE_H
