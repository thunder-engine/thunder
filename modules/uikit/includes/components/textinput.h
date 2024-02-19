#ifndef TEXTINPUT_H
#define TEXTINPUT_H

#include "widget.h"

class Label;
class Frame;

class ENGINE_EXPORT TextInput : public Widget {
    A_REGISTER(TextInput, Widget, Components/UI)

    A_PROPERTIES(
        A_PROPERTY(string, text, TextInput::text, TextInput::setText),
        A_PROPERTYEX(Vector4, textColor, TextInput::textColor, TextInput::setTextColor, "editor=Color"),
        A_PROPERTYEX(Vector4, backgroundColor, TextInput::backgroundColor, TextInput::setBackgroundColor, "editor=Color"),
        A_PROPERTYEX(Vector4, hoverColor, TextInput::hoverColor, TextInput::setHoverColor, "editor=Color"),
        A_PROPERTYEX(Vector4, pressedColor, TextInput::pressedColor, TextInput::setPressedColor, "editor=Color"),
        A_PROPERTYEX(Label *, textComponent, TextInput::textComponent, TextInput::setTextComponent, "editor=Component"),
        A_PROPERTYEX(Frame *, background, TextInput::background, TextInput::setBackground, "editor=Component")
    )
    A_METHODS(
        A_SIGNAL(TextInput::focusIn),
        A_SIGNAL(TextInput::focusOut),
        A_SIGNAL(TextInput::editingFinished)
    )
    A_NOENUMS()

public:
    TextInput();

    string text() const;
    void setText(const string text);

    Vector4 textColor() const;
    void setTextColor(Vector4 color);

    Vector4 backgroundColor() const;
    void setBackgroundColor(Vector4 color);

    Vector4 hoverColor() const;
    void setHoverColor(Vector4 color);

    Vector4 pressedColor() const;
    void setPressedColor(Vector4 color);

    Label *textComponent() const;
    void setTextComponent(Label *label);

    Frame *background() const;
    void setBackground(Frame *frame);

public: // signals
    void focusIn();
    void focusOut();

    void editingFinished();

protected:
    void update() override;

    void composeComponent() override;

    void onReferenceDestroyed() override;

    void recalcCursor();

private:
    Vector4 m_normalColor;
    Vector4 m_highlightedColor;
    Vector4 m_pressedColor;

    Vector4 m_textColor;

    Label *m_cursor;
    Label *m_label;

    Frame *m_background;

    int32_t m_cursorPosition;

    float m_fadeDuration;
    float m_currentFade;
    float m_cursorBlinkRate;
    float m_cursorBlinkCurrent;

    bool m_hovered;
    bool m_focused;

};

#endif // TEXTINPUT_H
