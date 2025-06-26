#ifndef NODEGROUP_H
#define NODEGROUP_H

#include <QColor>

#include <editor/graph/graphnode.h>

class NODEGRAPH_EXPORT NodeGroup : public GraphNode {
    A_OBJECT(NodeGroup, GraphNode, Graph)

    A_PROPERTIES(
        A_PROPERTY(string, text, NodeGroup::text, NodeGroup::setText),
        A_PROPERTYEX(Vector4, color, NodeGroup::groupColor, NodeGroup::setGroupColor, "editor=Color"),
        A_PROPERTY(float, width, NodeGroup::width, NodeGroup::setWidth),
        A_PROPERTY(float, height, NodeGroup::height, NodeGroup::setHeight)
    )

public:
    NodeGroup();

    std::string text() const;
    void setText(const std::string &text);

    Vector4 groupColor() const;
    void setGroupColor(const Vector4 &color);

    float width() const;
    void setWidth(const float width);

    float height() const;
    void setHeight(const float height);

protected:
    Vector2 defaultSize() const override;
    Vector4 color() const override;

    Widget *widget() override;

protected:
    Vector4 m_color;

    Vector2 m_size;

};

#endif // NODEGROUP_H
