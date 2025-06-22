#ifndef PASTENODES_H
#define PASTENODES_H

#include "../graphcontroller.h"

class PasteNodes : public UndoCommand {
public:
    PasteNodes(const std::string &data, int x, int y, GraphController *ctrl, const QString &name = QObject::tr("Paste Node"), QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    GraphController *m_controller;

    QDomDocument m_document;
    std::list<int32_t> m_list;
    std::list<int32_t> m_lastSelect;

    Vector2 m_pos;

};

#endif // PASTENODES_H
