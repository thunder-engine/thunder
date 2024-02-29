#ifndef WIDGET_H
#define WIDGET_H

#include <nativebehaviour.h>
#include <uikit.h>

class RectTransform;
class CommandBuffer;
class StyleSheet;

class UIKIT_EXPORT Widget : public NativeBehaviour {
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

    const list<string> &classes() const;
    void addClass(const string &name);

    Widget *parentWidget();
    list<Widget *> childWidgets() const;

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

    virtual void applyStyle();

    void setRectTransform(RectTransform *transform);

    void update() override;

    void actorParentChanged() override;

    void composeComponent() override;

    void setParent(Object *parent, int32_t position = -1, bool force = false) override;

    void drawGizmosSelected() override;

    float styleLength(const string &key, float value, bool &pixels);
    Vector4 styleBlockLength(const string &property, const Vector4 &value, bool &pixels);

    static void setFocusWidget(Widget *widget);

private:
    void setSystem(ObjectSystem *system) override;

    void addStyleRules(const map<string, string> &rules, uint32_t weight);

protected:
    map<string, pair<uint32_t, string>> m_styleRules;

private:
    friend class GuiLayer;
    friend class RectTransform;
    friend class UiLoader;
    friend class StyleSheet;

    list<string> m_classes;

    string m_style;

    Widget *m_parent;

    RectTransform *m_transform;

    static Widget *m_focusWidget;

    bool m_internal;

};

#endif // WIDGET_H
