#ifndef RECTTRANSFORM_H
#define RECTTRANSFORM_H

#include <components/transform.h>

class RectTransformPrivate;
class Widget;

class RectTransform : public Transform {
    A_REGISTER(RectTransform, Transform, General)

    A_PROPERTIES(
        A_PROPERTY(Vector2, size, RectTransform::size, RectTransform::setSize),
        A_PROPERTY(Vector2, pivot, RectTransform::pivot, RectTransform::setPivot),
        A_PROPERTY(Vector2, minAnchors, RectTransform::minAnchors, RectTransform::setMinAnchors),
        A_PROPERTY(Vector2, maxAnchors, RectTransform::maxAnchors, RectTransform::setMaxAnchors)
    )
    A_NOMETHODS()

    RectTransform();
    ~RectTransform();

    Vector2 size() const;
    void setSize(const Vector2 &size);

    Vector2 pivot() const;
    void setPivot(const Vector2 &pivot);

    Vector2 minAnchors() const;
    void setMinAnchors(const Vector2 &anchors);

    Vector2 maxAnchors() const;
    void setMaxAnchors(const Vector2 &anchors);

    bool isHovered(float x, float y) const;

    void subscribe(Widget *widget);
    void unsubscribe(Widget *widget);

private:
    RectTransformPrivate *p_ptr;
};

#endif // RECTTRANSFORM_H
