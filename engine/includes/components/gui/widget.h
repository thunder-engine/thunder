#ifndef WIDGET_H
#define WIDGET_H

#include "renderable.h"

class RectTransform;

class ENGINE_EXPORT Widget : public Renderable {
    A_REGISTER(Widget, Renderable, Components/UI)

    A_NOPROPERTIES()
    A_NOMETHODS()

public:
    Widget();
    ~Widget();

    Widget *parentWidget();

    RectTransform *rectTransform() const;

    static Widget *focusWidget();

protected:
    void setRectTransform(RectTransform *transform);

    virtual void boundChanged(const Vector2 &size);

    void update() override;

    void draw(CommandBuffer &buffer, uint32_t layer) override;

    AABBox bound() const override;

    void actorParentChanged() override;

    void composeComponent() override;

    void setParent(Object *parent, int32_t position = -1, bool force = false) override;

    static void setFocusWidget(Widget *widget);

#ifdef SHARED_DEFINE
    bool drawHandles(ObjectList &selected) override;
#endif

private:
    friend class RectTransform;

    Widget *m_parent;
    RectTransform *m_transform;

    static Widget *m_focusWidget;

};

#endif // WIDGET_H
