#ifndef LAYOUT_H
#define LAYOUT_H

#include "widget.h"

class ENGINE_EXPORT Layout {
public:
    enum Direction {
        Vertical,
        Horizontal
    };

public:
    Layout();
    ~Layout();

    void addTransform(RectTransform *transform);
    void removeTransform(RectTransform *transform);

    int indexOf(const Widget *widget) const;

    float spacing() const;
    void setSpacing(float spacing);

    void setMargins(float left, float top, float right, float bottom);

    int direction() const;
    void setDirection(int direction);

    Vector2 sizeHint() const;

    void invalidate();

    void update();

protected:
    friend class RectTransform;

    list<RectTransform *> m_widgets;

    Vector4 m_margins;

    RectTransform *m_parentTransform;

    float m_spacing;

    int m_direction;

    bool m_dirty;

};

#endif // LAYOUT_H
