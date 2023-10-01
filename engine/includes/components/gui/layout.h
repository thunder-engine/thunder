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

    void addLayout(Layout *layout);
    void addWidget(Widget *widget);

    void insertLayout(int index, Layout *layout);
    void insertWidget(int index, Widget *widget);

    void removeLayout(Layout *layout);
    void removeWidget(Widget *widget);

    int indexOf(const Layout *layout) const;
    int indexOf(const Widget *widget) const;

    int count() const;

    float spacing() const;
    void setSpacing(float spacing);

    void setMargins(float left, float top, float right, float bottom);

    int direction() const;
    void setDirection(int direction);

    Vector2 sizeHint() const;

    void invalidate();

    void update();

protected:
    list<Layout *> m_items;

    Vector4 m_margins;

    Vector2 m_position;

    Widget *m_attachedWidget;

    Layout *m_parentLayout;

    float m_spacing;

    int m_direction;

    bool m_dirty;

};

#endif // LAYOUT_H
