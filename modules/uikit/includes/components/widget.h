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

    std::string style() const;

    const std::list<std::string> &classes() const;
    void addClass(const std::string &name);

    Widget *parentWidget();
    std::list<Widget *> childWidgets() const;

    RectTransform *rectTransform() const;

    static Widget *focusWidget();

public: // slots
    void lower();

    void raise();

protected:
    virtual void draw(CommandBuffer &buffer);

    virtual void boundChanged(const Vector2 &size);

    virtual void applyStyle();

    void setRectTransform(RectTransform *transform);

    void actorParentChanged() override;

    void composeComponent() override;

    void setParent(Object *parent, int32_t position = -1, bool force = false) override;

    float styleLength(const std::string &key, float value, bool &pixels);
    Vector2 styleBlock2Length(const std::string &property, const Vector2 &value, bool &pixels);
    Vector4 styleBlock4Length(const std::string &property, const Vector4 &value, bool &pixels);

    static void setFocusWidget(Widget *widget);

private:
    void setSystem(ObjectSystem *system) override;

    void addStyleRules(const std::map<std::string, std::string> &rules, uint32_t weight);

protected:
    std::map<std::string, std::pair<uint32_t, std::string>> m_styleRules;

private:
    friend class GuiLayer;
    friend class RectTransform;
    friend class UiLoader;
    friend class StyleSheet;

    std::list<std::string> m_classes;

    Widget *m_parent;

    RectTransform *m_transform;

    static Widget *m_focusWidget;

};

#endif // WIDGET_H
