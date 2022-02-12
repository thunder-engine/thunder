#ifndef WIDGET_H
#define WIDGET_H

#include "renderable.h"
#include "gui.h"

class WidgetPrivate;

class RectTransform;

class GUI_EXPORT Widget : public Renderable {
    A_REGISTER(Widget, Renderable, Components/UI)

    A_NOPROPERTIES()
    A_NOMETHODS()

public:
    Widget();
    ~Widget();

    Widget *parentWidget();

    void setRectTransform(RectTransform *transform);

    void setParent(Object *parent, int32_t position = -1, bool force = false) override;

    virtual void boundChanged();

    void composeComponent() override;

protected:
    void update() override;

    void draw(CommandBuffer &buffer, uint32_t layer) override;

    AABBox bound() const override;

    void actorParentChanged() override;

#ifdef SHARED_DEFINE
    bool drawHandles(ObjectList &selected) override;
#endif

private:
    WidgetPrivate *p_ptr;

};

#endif // WIDGET_H
