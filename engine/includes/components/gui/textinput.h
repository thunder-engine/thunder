#ifndef TEXTINPUT_H
#define TEXTINPUT_H

#include "widget.h"

class Frame;
class Label;

class ENGINE_EXPORT TextInput : public Widget {
    A_REGISTER(TextInput, Widget, Components/UI)

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

protected:
    void update() override;

    void composeComponent() override;

    void onReferenceDestroyed() override;

    virtual void onClicked();

    void recalcCursor();

private:
    Vector4 m_normalColor;
    Vector4 m_highlightedColor;
    Vector4 m_pressedColor;

    Vector4 m_textColor;

    Frame *m_background;
    Label *m_cursor;
    Label *m_label;

    size_t m_cursorPosition;

    float m_fadeDuration;
    float m_currentFade;
    float m_cursorBlinkRate;
    float m_cursorBlinkCurrent;

    bool m_hovered;

};

#endif // TEXTINPUT_H
