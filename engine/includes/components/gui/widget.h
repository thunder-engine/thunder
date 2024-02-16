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

    string style() const;
    void setStyle(const string style);

    Widget *parentWidget();

    RectTransform *rectTransform() const;

    bool isInternal();

    void makeInternal();

    static Widget *focusWidget();

public: // slots
    void lower();

    void raise();

protected:
    virtual void draw(CommandBuffer &buffer);

    virtual void boundChanged(const Vector2 &size);

    void setRectTransform(RectTransform *transform);

    void update() override;

    void actorParentChanged() override;

    void composeComponent() override;

    void setParent(Object *parent, int32_t position = -1, bool force = false) override;

    void drawGizmosSelected() override;

    static void setFocusWidget(Widget *widget);

private:
    void setSystem(ObjectSystem *system) override;

private:
    friend class PipelineContext;
    friend class RectTransform;
    friend class UiLoader;

    string m_style;

    Widget *m_parent;
    RectTransform *m_transform;

    static Widget *m_focusWidget;

    bool m_internal;

};

#endif // WIDGET_H
