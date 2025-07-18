#ifndef DELETEMODULE_H
#define DELETEMODULE_H

#include "effectgraph.h"

#include <pugixml.hpp>

class EffectModule;
class ParticleEdit;

class DeleteModule : public UndoCommand {
public:
    DeleteModule(EffectModule *module, EffectGraph *graph, ParticleEdit *editor, const QString &name, QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    TString m_path;

    pugi::xml_document m_document;

    EffectGraph *m_graph;

    uint32_t m_object;
    uint32_t m_root;
    int32_t m_index;

};

#endif // DELETEMODULE_H
