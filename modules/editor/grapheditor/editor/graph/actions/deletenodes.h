#ifndef DELETENODES_H
#define DELETENODES_H

#include "../legacygraphcontroller.h"

#include <pugixml.hpp>

class DeleteNodes : public UndoCommand {
public:
    DeleteNodes(const std::list<int32_t> &selection, LegacyGraphController *ctrl, const TString &name = QObject::tr("Delete Node").toStdString(), UndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    std::list<int32_t> m_indices;
    pugi::xml_document m_document;

    LegacyGraphController *m_controller;

};

#endif // DELETENODES_H
