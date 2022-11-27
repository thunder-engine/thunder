#ifndef TEXTINPUT_H
#define TEXTINPUT_H

#include "widget.h"

class Frame;
class Label;

class ENGINE_EXPORT TextInput : public Widget {
    A_REGISTER(TextInput, Widget, Components/UI)

    A_METHODS(
        A_SIGNAL(TextInput::focusIn),
        A_SIGNAL(TextInput::focusOut),
        A_SIGNAL(TextInput::editingFinished)
    )

public:
    TextInput();

    Frame *background() const;
    void setBackground(Frame *frame);

    Label *textComponent() const;
    void setTextComponent(Label *label);

    string text() const;
    void setText(const string text);

    Vector4 textColor() const;
    void setTextColor(Vector4 color);

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

    Frame *m_background;
    Label *m_cursor;
    Label *m_label;

    int32_t m_cursorPosition;

    float m_fadeDuration;
    float m_currentFade;
    float m_cursorBlinkRate;
    float m_cursorBlinkCurrent;

    bool m_hovered;
    bool m_focused;

};

#endif // TEXTINPUT_H
