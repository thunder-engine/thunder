#ifndef LAYOUT_H
#define LAYOUT_H

#include "widget.h"

class UIKIT_EXPORT Layout {
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

    RectTransform *transformAt(int index);

    RectTransform *rectTransform();
    void setRectTransform(RectTransform *transform);

    int count() const;

    int spacing() const;
    void setSpacing(int spacing);

    int orientation() const;
    void setOrientation(int orientation);

    Vector2 sizeHint();

    void invalidate();

    void update();

protected:
    std::list<Layout *> m_items;

    Vector2 m_position;

    Layout *m_parentLayout;

    RectTransform *m_attachedTransform;

    RectTransform *m_rectTransform;

    int m_spacing;

    int m_orientation;

    bool m_dirty;

};

#endif // LAYOUT_H
