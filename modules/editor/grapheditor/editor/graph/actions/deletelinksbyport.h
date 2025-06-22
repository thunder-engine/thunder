#ifndef DELETELINKSBYPORT_H
#define DELETELINKSBYPORT_H

#include "../graphcontroller.h"

class DeleteLinksByPort : public UndoCommand {
public:
    DeleteLinksByPort(int node, int port, GraphController *ctrl, const QString &name = QObject::tr("Delete Links"), QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    GraphController *m_controller;

    int m_node;
    int m_port;

    struct Link {
        int sender;
        int oport;
        int receiver;
        int iport;
    };

    std::list<Link> m_links;
};

#endif // DELETELINKSBYPORT_H
