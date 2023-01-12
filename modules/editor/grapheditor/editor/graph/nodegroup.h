#ifndef NODEGROUP_H
#define NODEGROUP_H

#include <QColor>

#include <editor/graph/graphnode.h>

class NODEGRAPH_EXPORT NodeGroup : public GraphNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "")

    Q_PROPERTY(QString text READ objectName WRITE setObjectName DESIGNABLE true USER true)
    Q_PROPERTY(QColor color READ groupColor WRITE setGroupColor DESIGNABLE true USER true)
    Q_PROPERTY(float width READ width WRITE setWidth DESIGNABLE true USER true)
    Q_PROPERTY(float height READ height WRITE setHeight DESIGNABLE true USER true)

public:
    Q_INVOKABLE NodeGroup();

    QColor groupColor() const;
    void setGroupColor(const QColor &color);

    float width() const;
    void setWidth(const float width);

    float height() const;
    void setHeight(const float height);

protected:
    Vector2 defaultSize() const override;
    Vector4 color() const override;

protected:
    QColor m_color;

    Vector2 m_size;

};

#endif // NODEGROUP_H
