#ifndef CREATEMODULE_H
#define CREATEMODULE_H

#include "effectgraph.h"

class ParticleEdit;

class CreateModule : public UndoCommand {
public:
    CreateModule(const TString &module, EffectGraph *graph, const TString &name, UndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    TString m_moduleName;

    EffectGraph *m_graph;

    uint32_t m_object;

};

#endif // CREATEMODULE_H
