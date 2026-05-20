#ifndef CHECKBOX_H
#define CHECKBOX_H

#include "abstractbutton.h"

class UIKIT_EXPORT CheckBox : public AbstractButton {
    A_OBJECT(CheckBox, AbstractButton, Components/UI)

    A_PROPERTIES(
        A_PROPERTYEX(Sprite *, indicator, CheckBox::indicator, CheckBox::setIndicator, "editor=Asset"),
        A_PROPERTYEX(Vector4, indicatorColor, CheckBox::indicatorColor, CheckBox::setIndicatorColor, "editor=Color"),
        A_PROPERTYEX(Vector2, iconSize, CheckBox::indicatorSize, CheckBox::setIndicatorSize, "css=indicator-size")
    )
    A_NOMETHODS()
    A_NOENUMS()

public:
    CheckBox();

    Sprite *indicator() const;
    void setIndicator(Sprite *icon);

    Vector4 indicatorColor() const;
    void setIndicatorColor(const Vector4 color);

    Vector2 indicatorSize() const;
    void setIndicatorSize(const Vector2 &size);

    void setFoldMode(bool fold);

    void draw() override;

protected:
    void composeComponent() override;

    void applyStyle() override;

private:
    Vector4 m_knobColor;
    Vector2 m_knobSize;

    Sprite *m_knobIcon;

    Mesh *m_iconMesh;

    MaterialInstance *m_iconMaterial;

    bool m_foldMode;
    bool m_dirtyIcon;

};

#endif // CHECKBOX_H
