#ifndef DELETELINKS_H
#define DELETELINKS_H

#include "../graphcontroller.h"

#include <pugixml.hpp>

class DeleteLinks : public UndoCommand {
public:
    DeleteLinks(const std::list<int32_t> &links, GraphController *ctrl, const TString &name = "Delete Links", UndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    std::list<int32_t> m_indices;
    pugi::xml_document m_document;

    GraphController *m_controller;
};

#endif // DELETELINKS_H
