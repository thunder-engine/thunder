#ifndef LAYOUT_H
#define LAYOUT_H

#include "widget.h"

class UIKIT_EXPORT Layout {
public:
    Layout();
    ~Layout();

    void addTransform(RectTransform *transform);

    void insertTransform(int index, RectTransform *transform);

    void removeTransform(RectTransform *transform);

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

    void solveItemsDimension(int availableSpace, bool horizontal);
    void solveItemsPosition(float height, const Vector2 &offset);

protected:
    std::list<RectTransform *> m_items;

    RectTransform *m_rectTransform;

    int m_spacing;

    int m_orientation;
};

#endif // LAYOUT_H
