#include "nodegroup.h"

NodeGroup::NodeGroup() :
        m_color(QColor(255, 255, 255, 255)),
        m_size(Vector2(400.0f, 200.0f)) {

}

QColor NodeGroup::groupColor() const {
    return m_color;
}
void NodeGroup::setGroupColor(const QColor &color) {
    m_color = color;
}

float NodeGroup::width() const {
    return m_size.x;
}
void NodeGroup::setWidth(const float width) {
    m_size.x = width;
}

float NodeGroup::height() const {
    return m_size.y;
}
void NodeGroup::setHeight(const float height) {
    m_size.y = height;
}

Vector2 NodeGroup::defaultSize() const {
    return m_size;
}

Vector4 NodeGroup::color() const {
    return Vector4(m_color.redF(), m_color.greenF(),
                   m_color.blueF(), m_color.alphaF());
}
