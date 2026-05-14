#ifndef LINEEDIT_H
#define LINEEDIT_H

#include "frame.h"

#include <font.h>

class Label;
class Sprite;

class UIKIT_EXPORT LineEdit : public Widget {
    A_OBJECT(LineEdit, Widget, Components/UI)

    A_PROPERTIES(
        A_PROPERTY(TString, text, LineEdit::text, LineEdit::setText),
        A_PROPERTYEX(Font *, font, LineEdit::font, LineEdit::setFont, "editor=Asset"),
        A_PROPERTYEX(Vector4, textColor, LineEdit::textColor, LineEdit::setTextColor, "editor=Color")
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

    Font *font() const;
    void setFont(Font *font);

    Vector4 textColor() const;
    void setTextColor(const Vector4 &color);

    Vector4 backgroundColor() const;
    void setBackgroundColor(const Vector4 &color);

public: // signals
    void focusIn();
    void focusOut();

    void editingFinished();

protected:
    void update(const Vector2 &pos) override;

    void composeComponent() override;

    void recalcCursor();

    static void fontUpdated(int state, void *ptr);

    float cursorAt(int position) const;

private:
    void draw() override;

private:
    TString m_text;

    Vector4 m_backgroundColor;

    Vector4 m_textColor;

    Matrix4 m_cursorTransform;

    MaterialInstance *m_cursorMaterial;
    MaterialInstance *m_fontMaterial;
    MaterialInstance *m_spriteMaterial;
    MaterialInstance *m_frameMaterial;

    Font *m_font;
    Sprite *m_backgroundImage;

    Mesh *m_textMesh;
    Mesh *m_backgroundMesh;

    int32_t m_cursorPosition;
    int32_t m_fontSize;

    float m_textPosition;
    float m_cursorBlinkRate;
    float m_cursorBlinkCurrent;
    float m_cursorRepeatHold;

    bool m_hovered;
    bool m_focused;
    bool m_cursorVisible;

    bool m_textDirty;

};

#endif // LINEEDIT_H
