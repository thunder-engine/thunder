#ifndef BUTTON_H
#define BUTTON_H

#include "abstractbutton.h"

#include <font.h>

class UIKIT_EXPORT Button : public AbstractButton {
    A_OBJECT(Button, AbstractButton, Components/UI)

    A_PROPERTIES(
        A_PROPERTY(TString, text, Button::text, Button::setText),
        A_PROPERTYEX(Font *, font, Button::font, Button::setFont, "editor=Asset"),
        A_PROPERTYEX(int, fontSize, Button::fontSize, Button::setFontSize, "css=font-size"),
        A_PROPERTYEX(Vector4, textColor, Button::textColor, Button::setTextColor, "editor=Color"),
        A_PROPERTYEX(Sprite *, icon, Button::icon, Button::setIcon, "editor=Asset"),
        A_PROPERTYEX(Vector2, iconSize, Button::iconSize, Button::setIconSize, "css=icon-size")
    )
    A_NOMETHODS()
    A_NOENUMS()

public:
    Button();
    ~Button();

    TString text() const;
    void setText(const TString &text);

    Font *font() const;
    void setFont(Font *font);

    int fontSize() const;
    void setFontSize(int size);

    Vector4 textColor() const;
    void setTextColor(const Vector4 &color);

    Sprite *icon() const;
    void setIcon(Sprite *icon);

    Vector2 iconSize() const;
    void setIconSize(const Vector2 &size);

    void setIconRotation(float angle);

    void draw() override;

protected:
    void applyStyle() override;

    static void fontUpdated(int state, void *ptr);

protected:
    TString m_text;

    Vector4 m_textColor;
    Vector2 m_iconSize;

    Sprite *m_icon;

    Font *m_font;

    Mesh *m_iconMesh;
    Mesh *m_textMesh;

    MaterialInstance *m_fontMaterial;
    MaterialInstance *m_iconMaterial;

    int32_t m_fontSize;

    float m_rotation;

    bool m_dirtyIcon;
    bool m_dirtyText;

};

#endif // BUTTON_H
