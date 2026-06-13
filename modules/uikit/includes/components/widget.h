#ifndef WIDGET_H
#define WIDGET_H

#include <component.h>
#include <uikit.h>

class RectTransform;
class CommandBuffer;
class StyleSheet;

class Canvas;

class KeyEvent {
public:
    KeyEvent(int keyCode, bool isPressed, bool isRepeat = false);

    int keyCode() const;
    bool isPressed() const;
    bool isRepeat() const;

    bool isShiftPressed() const;
    bool isControlPressed() const;
    bool isAltPressed() const;

private:
    int m_keyCode;
    bool m_isPressed;
    bool m_isRepeat;
};

class UIKIT_EXPORT Widget : public Component {
    A_OBJECT(Widget, Component, Components/UI)

    A_NOPROPERTIES()
    A_METHODS(
        A_METHOD(RectTransform *, Widget::rectTransform),
        A_SLOT(Widget::lower),
        A_SLOT(Widget::raise)
    )
    A_ENUMS(
        A_ENUM(Orientation,
            A_VALUE(Horizontal),
            A_VALUE(Vertical)
        )
    )

public:
    enum Orientation {
        Horizontal,
        Vertical
    };

public:
    Widget();
    ~Widget();

    TString style() const;

    const StringList &classes() const;
    void addClass(const TString &name);

    Widget *parentWidget() const;
    std::list<Widget *> &childWidgets();

    RectTransform *rectTransform();

    Canvas *canvas();

    bool isSubWidget() const;

    virtual void draw();

    virtual bool isHovered(const Vector2 &pos);

    virtual void update(const Vector2 &pos);

    static Widget *focusWidget();

    void repaint();

    void setEnabled(bool enable) override;

    virtual bool onMouseDown(int x, int y) { return false; }
    virtual bool onMouseUp(int x, int y) { return false; }
    virtual bool onMouseMove(int x, int y) { return false; }
    virtual bool onMouseDoubleClick(int x, int y) { return false; }
    virtual bool onMouseWheel(int delta, bool horizontal) { return false; }
    virtual bool onKeyPress(KeyEvent *event) { return false; }
    virtual bool onKeyRelease(KeyEvent *event) { return false; }

    Widget *subWidget(const TString &name) const;
    void setSubWidget(Widget *widget);

public: // slots
    void lower();

    void raise();

protected:
    virtual void drawSub();

    virtual void boundChanged(const Vector2 &size);

    virtual void applyStyle();

    void setRectTransform(RectTransform *transform);

    void onHierarchyUpdated();

    void composeComponent() override;

    void dispatchKeyEvent(KeyEvent *event);
    void dispatchMouseEvent(const Vector2 &pos, Event::Type type, int button);
    void dispatchMouseWheelEvent(const Vector2 &pos, int delta, bool horizontal);

    float styleLength(const TString &key, float value, bool &pixels);
    Vector2 styleBlock2Length(const TString &property, const Vector2 &value, bool &pixels);
    Vector4 styleBlock4Length(const TString &property, const Vector4 &value, bool &pixels);

    static void setFocusWidget(Widget *widget);

    void updateStyleProperty(const TString &name, const float *v, int32_t size);

protected:
    virtual void childAdded(RectTransform *child);

protected:
    std::map<TString, std::pair<uint32_t, TString>> m_styleRules;

    std::list<Widget *> m_childWidgets;

private:
    void addStyleRules(const std::map<TString, TString> &rules, uint32_t weight);

private:
    friend class RectTransform;
    friend class Canvas;
    friend class StyleSheet;

    StringList m_classes;

    Widget *m_parent;

    Canvas *m_canvas;

    RectTransform *m_transform;

    bool m_subWidget;

    static Widget *m_focusWidget;

};

#endif // WIDGET_H
