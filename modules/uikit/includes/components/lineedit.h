#ifndef LINEEDIT_H
#define LINEEDIT_H

#include "frame.h"

class Label;

class UIKIT_EXPORT LineEdit : public Frame {
    A_OBJECT(LineEdit, Frame, Components/UI)

    A_PROPERTIES(
        A_PROPERTY(TString, text, LineEdit::text, LineEdit::setText),
        A_PROPERTYEX(Vector4, textColor, LineEdit::textColor, LineEdit::setTextColor, "editor=Color"),
        A_PROPERTYEX(Label *, textComponent, LineEdit::textComponent, LineEdit::setTextComponent, "editor=Component")
    )
    A_METHODS(
        A_SIGNAL(LineEdit::focusIn),
        A_SIGNAL(LineEdit::focusOut),
        A_SIGNAL(LineEdit::editingFinished)
    )
    A_NOENUMS()

public:
    LineEdit();

    TString text() const;
    void setText(const TString &text);

    Vector4 textColor() const;
    void setTextColor(const Vector4 &color);

    Label *textComponent() const;
    void setTextComponent(Label *label);

public: // signals
    void focusIn();
    void focusOut();

    void editingFinished();

protected:
    void update() override;

    void composeComponent() override;

    void recalcCursor();

private:
    void drawSub(CommandBuffer &buffer) override;

private:
    TString m_text;

    Vector4 m_normalColor;
    Vector4 m_highlightedColor;
    Vector4 m_pressedColor;

    Vector4 m_textColor;

    Matrix4 m_cursorTransform;

    Mesh *m_cursor;

    MaterialInstance *m_cursorMaterial;

    int32_t m_cursorPosition;

    float m_fadeDuration;
    float m_currentFade;
    float m_cursorBlinkRate;
    float m_cursorBlinkCurrent;
    float m_cursorRepeatHold;

    bool m_hovered;
    bool m_focused;
    bool m_cursorVisible;

};

#endif // LINEEDIT_H
