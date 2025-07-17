#ifndef WIDGET_H
#define WIDGET_H

#include <nativebehaviour.h>
#include <uikit.h>

class RectTransform;
class CommandBuffer;
class StyleSheet;

class UIKIT_EXPORT Widget : public NativeBehaviour {
    A_OBJECT(Widget, NativeBehaviour, Components/UI)

    A_NOPROPERTIES()
    A_METHODS(
        A_METHOD(RectTransform *, Widget::rectTransform),
        A_SLOT(Widget::lower),
        A_SLOT(Widget::raise)
    )

public:
    Widget();
    ~Widget();

    TString style() const;

    const StringList &classes() const;
    void addClass(const TString &name);

    Widget *parentWidget() const;
    std::list<Widget *> childWidgets() const;

    RectTransform *rectTransform() const;

    bool isSubWidget(Widget *widget) const;

    static Widget *focusWidget();

public: // slots
    void lower();

    void raise();

protected:
    virtual void draw(CommandBuffer &buffer);

    virtual void boundChanged(const Vector2 &size);

    virtual void applyStyle();

    void setRectTransform(RectTransform *transform);

    void onReferenceDestroyed() override;

    void actorParentChanged() override;

    void composeComponent() override;

    void setParent(Object *parent, int32_t position = -1, bool force = false) override;

    float styleLength(const TString &key, float value, bool &pixels);
    Vector2 styleBlock2Length(const TString &property, const Vector2 &value, bool &pixels);
    Vector4 styleBlock4Length(const TString &property, const Vector4 &value, bool &pixels);

    Widget *subWidget(const TString &name) const;
    void setSubWidget(const TString &name, Widget *widget);

    static void setFocusWidget(Widget *widget);

private:
    void addStyleRules(const std::map<TString, TString> &rules, uint32_t weight);

    void setSystem(ObjectSystem *system) override;

protected:
    std::map<TString, std::pair<uint32_t, TString>> m_styleRules;

    std::map<TString, Widget *> m_subWidgets;

private:
    friend class GuiLayer;
    friend class RectTransform;
    friend class UiLoader;
    friend class StyleSheet;

    StringList m_classes;

    Widget *m_parent;

    RectTransform *m_transform;

    static Widget *m_focusWidget;

};

#endif // WIDGET_H
