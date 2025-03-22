#ifndef GROUPWIDGET_H
#define GROUPWIDGET_H

#include "nodewidget.h"

class GraphNode;
class Label;

class GroupWidget : public NodeWidget {
    A_OBJECT(GroupWidget, NodeWidget, Editor/Graph)

    enum Points {
        POINT_T = (1 << 0),
        POINT_B = (1 << 1),
        POINT_L = (1 << 2),
        POINT_R = (1 << 3)
    };

public:
    GroupWidget();

    int cursorShape() const;

private:
    void update() override;

    void composeComponent() override;

    Vector2 size() const;
    void setSize(const Vector2 &size);

private:
    int m_shape;

    int m_resize;

};

#endif // GROUPWIDGET_H
