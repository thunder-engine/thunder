#ifndef RECTTRANSFORM_H
#define RECTTRANSFORM_H

#include <transform.h>

class Widget;
class Layout;

class ENGINE_EXPORT RectTransform : public Transform {
    A_REGISTER(RectTransform, Transform, General)

    A_PROPERTIES(
        A_PROPERTY(Vector2, size, RectTransform::size, RectTransform::setSize),
        A_PROPERTY(Vector2, minAnchors, RectTransform::minAnchors, RectTransform::setMinAnchors),
        A_PROPERTY(Vector2, maxAnchors, RectTransform::maxAnchors, RectTransform::setMaxAnchors),
        A_PROPERTY(Vector2, offsetMin, RectTransform::offsetMin, RectTransform::setOffsetMin),
        A_PROPERTY(Vector2, offsetMax, RectTransform::offsetMax, RectTransform::setOffsetMax),
        A_PROPERTY(Vector2, pivot, RectTransform::pivot, RectTransform::setPivot)
    )
    A_NOMETHODS()

    RectTransform();
    ~RectTransform();

    Vector2 size() const;
    void setSize(const Vector2 size);

    Vector2 pivot() const;
    void setPivot(const Vector2 pivot);

    Vector2 minAnchors() const;
    void setMinAnchors(const Vector2 anchors);

    Vector2 maxAnchors() const;
    void setMaxAnchors(const Vector2 anchors);

    void setAnchors(const Vector2 minimum, const Vector2 maximum);

    Vector2 offsetMin() const;
    void setOffsetMin(const Vector2 offset);

    Vector2 offsetMax() const;
    void setOffsetMax(const Vector2 offset);

    void setOffsets(const Vector2 minimum, const Vector2 maximum);

    bool isHovered(float x, float y) const;

    void subscribe(Widget *widget);
    void unsubscribe(Widget *widget);

    Layout *layout() const;
    void setLayout(Layout *layout);

    Matrix4 worldTransform() const override;

private:
    void setDirty(bool dirty) override;

    void cleanDirty() const override;

    void notify();

    void recalcSize();

private:
    Vector2 m_bottomLeft;
    Vector2 m_topRight;
    Vector2 m_pivot;
    Vector2 m_minAnchors;
    Vector2 m_maxAnchors;
    Vector2 m_size;
    list<Widget *> m_subscribers;

    Layout *m_layout;

};

#endif // RECTTRANSFORM_H
