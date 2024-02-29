#ifndef LAYOUT_H
#define LAYOUT_H

#include "widget.h"

class UIKIT_EXPORT Layout {
public:
    enum Direction {
        Vertical,
        Horizontal
    };

public:
    Layout();
    ~Layout();

    void addLayout(Layout *layout);
    void addTransform(RectTransform *transform);

    void insertLayout(int index, Layout *layout);
    void insertTransform(int index, RectTransform *transform);

    void removeLayout(Layout *layout);
    void removeTransform(RectTransform *transform);

    int indexOf(const Layout *layout) const;
    int indexOf(const RectTransform *transform) const;

    RectTransform *rectTransform();
    void setRectTransform(RectTransform *transform);

    int count() const;

    float spacing() const;
    void setSpacing(float spacing);

    int direction() const;
    void setDirection(int direction);

    Vector2 sizeHint() const;

    void invalidate();

    void update();

protected:
    list<Layout *> m_items;

    Vector2 m_position;

    Layout *m_parentLayout;

    RectTransform *m_attachedTransform;

    RectTransform *m_rectTransform;

    float m_spacing;

    int m_direction;

    bool m_dirty;

};

#endif // LAYOUT_H
