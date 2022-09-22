#ifndef ABSTRACTBUTTON_H
#define ABSTRACTBUTTON_H

#include "widget.h"

class Frame;
class Label;

class GUI_EXPORT AbstractButton : public Widget {
    A_REGISTER(AbstractButton, Widget, General)

    A_PROPERTIES(
        A_PROPERTY(string, text, AbstractButton::text, AbstractButton::setText),
        A_PROPERTY(bool, checkable, AbstractButton::isCheckable, AbstractButton::setCheckable),
        A_PROPERTY(bool, checked, AbstractButton::isChecked, AbstractButton::setChecked),
        A_PROPERTY(bool, exclusive, AbstractButton::isExclusive, AbstractButton::setExclusive),
        A_PROPERTYEX(Frame *, background, AbstractButton::background, AbstractButton::setBackground, "editor=Component"),
        A_PROPERTYEX(Label *, label, AbstractButton::label, AbstractButton::setLabel, "editor=Component"),
        A_PROPERTY(float, fadeDuration, AbstractButton::fadeDuration, AbstractButton::setFadeDuration),
        A_PROPERTYEX(Vector4, highlightedColor, AbstractButton::highlightedColor, AbstractButton::setHighlightedColor, "editor=Color"),
        A_PROPERTYEX(Vector4, normalColor, AbstractButton::normalColor, AbstractButton::setNormalColor, "editor=Color"),
        A_PROPERTYEX(Vector4, pressedColor, AbstractButton::pressedColor, AbstractButton::setPressedColor, "editor=Color")
    )
    A_METHODS(
        A_SIGNAL(AbstractButton::clicked)
    )

public:
    AbstractButton();
    ~AbstractButton();

    string text() const;
    void setText(const string text);

    Frame *background() const;
    void setBackground(Frame *frame);

    Label *label() const;
    void setLabel(Label *label);

    float fadeDuration() const;
    void setFadeDuration(float duration);

    Vector4 highlightedColor() const;
    void setHighlightedColor(const Vector4 color);

    Vector4 normalColor() const;
    void setNormalColor(const Vector4 color);

    Vector4 pressedColor() const;
    void setPressedColor(const Vector4 color);

    bool isCheckable() const;
    void setCheckable(bool checkable);

    bool isChecked() const;
    void setChecked(bool checked);

    bool isExclusive() const;
    void setExclusive(bool exclusive);

    bool isMirrored() const;
    virtual void setMirrored(bool flag);

    void clicked();

protected:
    void onReferenceDestroyed() override;

    void composeComponent() override;

    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

    void update() override;

    virtual void onClicked();

    virtual void checkStateSet();

protected:
    string m_text;

    Vector4 m_normalColor;
    Vector4 m_highlightedColor;
    Vector4 m_pressedColor;

    Label *m_label;
    Frame *m_background;

    float m_fadeDuration;
    float m_currentFade;

    bool m_hovered;
    bool m_mirrored;
    bool m_checkable;
    bool m_checked;
    bool m_exclusive;

};

#endif // ABSTRACTBUTTON_H
