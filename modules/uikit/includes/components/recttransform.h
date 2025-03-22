#ifndef RECTTRANSFORM_H
#define RECTTRANSFORM_H

#include <transform.h>
#include <uikit.h>

class Widget;
class Layout;

class UIKIT_EXPORT RectTransform : public Transform {
    A_OBJECT(RectTransform, Transform, General)

    A_PROPERTIES(
        A_PROPERTY(Vector2, size, RectTransform::size, RectTransform::setSize),
        A_PROPERTY(Vector2, minAnchors, RectTransform::minAnchors, RectTransform::setMinAnchors),
        A_PROPERTY(Vector2, maxAnchors, RectTransform::maxAnchors, RectTransform::setMaxAnchors),
        A_PROPERTY(Vector2, pivot, RectTransform::pivot, RectTransform::setPivot),
        A_PROPERTY(Vector4, margin, RectTransform::margin, RectTransform::setMargin),
        A_PROPERTY(Vector4, boder, RectTransform::border, RectTransform::setBorder),
        A_PROPERTY(Vector4, padding, RectTransform::padding, RectTransform::setPadding),
        A_PROPERTY(bool, mouseTracking, RectTransform::mouseTracking, RectTransform::setMouseTracking)
    )
    A_NOMETHODS()

    enum SizePolicy {
        Fixed = 0,
        Minimum,
        Maximum,
        Preferred
    };

public:
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

    Vector4 margin() const;
    void setMargin(const Vector4 margin);

    Vector4 border() const;
    void setBorder(const Vector4 border);

    Vector4 padding() const;
    void setPadding(const Vector4 padding);

    bool mouseTracking() const;
    void setMouseTracking(bool tracking);

    bool isHovered(float x, float y) const;

    RectTransform *hoveredTransform(float x, float y);

    void subscribe(Widget *widget);
    void unsubscribe(Widget *widget);

    Layout *layout() const;
    void setLayout(Layout *layout);

    AABBox bound() const;

    void setEnabled(bool enabled) override;

    SizePolicy verticalPolicy() const;
    void setVerticalPolicy(SizePolicy policy);

    SizePolicy horizontalPolicy() const;
    void setHorizontalPolicy(SizePolicy policy);

private:
    friend class Layout;

    void cleanDirty() const override;

    void recalcChilds() const;

    void recalcParent();

private:
    std::list<Widget *> m_subscribers;

    Vector4 m_margin;
    Vector4 m_border;
    Vector4 m_padding;

    Vector2 m_pivot;
    Vector2 m_minAnchors;
    Vector2 m_maxAnchors;
    mutable Vector2 m_size;

    Layout *m_layout;
    Layout *m_attachedLayout;

    SizePolicy m_verticalPolicy;
    SizePolicy m_horizontalPolicy;

    bool m_mouseTracking;

    mutable bool m_flag = false;

};

#endif // RECTTRANSFORM_H
