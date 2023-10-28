#ifndef TEXTINPUT_H
#define TEXTINPUT_H

#include "frame.h"

class Label;

class ENGINE_EXPORT TextInput : public Frame {
    A_REGISTER(TextInput, Frame, Components/UI)

    A_NOPROPERTIES()
    A_METHODS(
        A_SIGNAL(TextInput::focusIn),
        A_SIGNAL(TextInput::focusOut),
        A_SIGNAL(TextInput::editingFinished)
    )
    A_NOENUMS()

public:
    TextInput();

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
