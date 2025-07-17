#ifndef DELETENODES_H
#define DELETENODES_H

#include "../graphcontroller.h"

#include <pugixml.hpp>

class DeleteNodes : public UndoCommand {
public:
    DeleteNodes(const std::list<int32_t> &selection, GraphController *ctrl, const QString &name = QObject::tr("Delete Node"), QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    std::list<int32_t> m_indices;
    pugi::xml_document m_document;

    GraphController *m_controller;

};

#endif // DELETENODES_H
