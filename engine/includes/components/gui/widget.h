#ifndef WIDGET_H
#define WIDGET_H

#include "../nativebehaviour.h"

class RectTransform;
class CommandBuffer;

class ENGINE_EXPORT Widget : public NativeBehaviour {
    A_REGISTER(Widget, NativeBehaviour, Components/UI)

    A_NOPROPERTIES()
    A_NOMETHODS()

public:
    Widget();
    ~Widget();

    Widget *parentWidget();

    RectTransform *rectTransform() const;

    static Widget *focusWidget();

    virtual void draw(CommandBuffer &buffer, uint32_t layer);

protected:
    void setRectTransform(RectTransform *transform);

    virtual void boundChanged(const Vector2 &size);

    void update() override;

    virtual AABBox bound() const;

    void actorParentChanged() override;

    void composeComponent() override;

    void setParent(Object *parent, int32_t position = -1, bool force = false) override;

    static void setFocusWidget(Widget *widget);

#ifdef SHARED_DEFINE
    bool drawHandles(ObjectList &selected) override;
#endif

private:
    void setSystem(ObjectSystem *system) override;

private:
    friend class RectTransform;

    Widget *m_parent;
    RectTransform *m_transform;

    static Widget *m_focusWidget;

};

#endif // WIDGET_H
