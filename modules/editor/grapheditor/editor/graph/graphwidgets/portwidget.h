#ifndef PORTWIDGET_H
#define PORTWIDGET_H

#include <components/frame.h>

class NodePort;

class PortWidget : public Widget {
    A_REGISTER(PortWidget, Widget, Editor/UI)

public:
    PortWidget();

    void setNodePort(NodePort *port);

private:
    void composeComponent() override;

    bool drawHandles(ObjectList &selected) override;

private:
    NodePort *m_port;

    Actor *m_editor;
};

#endif // PORTWIDGET_H
