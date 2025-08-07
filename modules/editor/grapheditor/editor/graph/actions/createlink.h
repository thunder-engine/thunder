#ifndef CREATELINK_H
#define CREATELINK_H

#include "../graphcontroller.h"

class CreateLink : public UndoCommand {
public:
    CreateLink(int sender, int oport, int receiver, int iport, GraphController *ctrl, const TString &name = QObject::tr("Create Link").toStdString(), UndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    GraphController *m_controller;

    int m_sender;
    int m_oPort;
    int m_receiver;
    int m_iPort;

    int m_index;
};

#endif // CREATELINK_H
