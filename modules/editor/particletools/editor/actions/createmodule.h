#ifndef CREATEMODULE_H
#define CREATEMODULE_H

#include "effectgraph.h"

class CreateModule : public UndoCommand {
public:
    CreateModule(const std::string &module, EffectGraph *graph, const QString &name, QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    std::string m_moduleName;

    EffectGraph *m_graph;

    uint32_t m_object;

};

#endif // CREATEMODULE_H
