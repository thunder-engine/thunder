#ifndef WIDGET_H
#define WIDGET_H

#include "../nativebehaviour.h"

class RectTransform;
class CommandBuffer;

class ENGINE_EXPORT Widget : public NativeBehaviour {
    A_REGISTER(Widget, NativeBehaviour, Components/UI)

    A_NOPROPERTIES()
    A_METHODS(
        A_METHOD(RectTransform *, Widget::rectTransform),
        A_SLOT(Widget::lower),
        A_SLOT(Widget::raise)
    )

public:
    Widget();
    ~Widget();

    Widget *parentWidget();

    RectTransform *rectTransform() const;

    static Widget *focusWidget();

    virtual void draw(CommandBuffer &buffer, uint32_t layer);

public: // slots
    void lower();

    void raise();

protected:
    virtual void boundChanged(const Vector2 &size);

    virtual AABBox bound() const;

    void setRectTransform(RectTransform *transform);

    void update() override;

    void actorParentChanged() override;

    void composeComponent() override;

    void setParent(Object *parent, int32_t position = -1, bool force = false) override;

    static void setFocusWidget(Widget *widget);

    void drawGizmosSelected() override;

private:
    void setSystem(ObjectSystem *system) override;

private:
    friend class RectTransform;

    Widget *m_parent;
    RectTransform *m_transform;

    static Widget *m_focusWidget;

};

#endif // WIDGET_H
