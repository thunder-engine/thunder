#ifndef PORTWIDGET_H
#define PORTWIDGET_H

#include <components/gui/frame.h>

class NodePort;

class PortWidget : public Widget {
    A_REGISTER(PortWidget, Widget, Editor/Graph)

    A_METHODS(
        A_SIGNAL(PortWidget::pressed),
        A_SIGNAL(PortWidget::released)
    )

public:
    PortWidget();

    void portUpdate();

    NodePort *port() const;
    void setNodePort(NodePort *port);

    Frame *knob() const;

public:
    void pressed(int position);

    void released(int position);

private:
    void update() override;

    void composeComponent() override;

private:
    NodePort *m_port;

    Actor *m_label;
    Actor *m_editor;

    Frame *m_knob;

    bool m_hovered;

};

#endif // PORTWIDGET_H
