#ifndef ABSTRACTBUTTON_H
#define ABSTRACTBUTTON_H

#include "widget.h"

#include <resources/sprite.h>

class MaterialInstance;

class UIKIT_EXPORT AbstractButton : public Widget {
    A_OBJECT(AbstractButton, Widget, General)

    A_PROPERTIES(
        A_PROPERTY(bool, checkable, AbstractButton::isCheckable, AbstractButton::setCheckable),
        A_PROPERTY(bool, checked, AbstractButton::isChecked, AbstractButton::setChecked),
        A_PROPERTY(bool, exclusive, AbstractButton::isExclusive, AbstractButton::setExclusive),
        A_PROPERTY(Vector4, corners, AbstractButton::corners, AbstractButton::setCorners),
        A_PROPERTYEX(Vector4, backgroundColor, AbstractButton::backgroundColor, AbstractButton::setBackgroundColor, "editor=Color, css=background-color"),
        A_PROPERTYEX(Vector4, borderColor, AbstractButton::borderColor, AbstractButton::setBorderColor, "editor=Color, css=border-color"),
        A_PROPERTYEX(Sprite *, backgroundImage, AbstractButton::backgroundImage, AbstractButton::setBackgroundImage, "editor=Asset")
    )
    A_METHODS(
        A_SIGNAL(AbstractButton::pressed),
        A_SIGNAL(AbstractButton::clicked),
        A_SIGNAL(AbstractButton::toggled)
    )
    A_NOENUMS()

public:
    AbstractButton();

    Sprite *backgroundImage() const;
    void setBackgroundImage(Sprite *image);

    Vector4 backgroundColor() const;
    void setBackgroundColor(const Vector4 &color);

    Vector4 highlightedColor() const;
    void setHighlightedColor(const Vector4 &color);

    Vector4 pressedColor() const;
    void setPressedColor(const Vector4 &color);

    Vector4 borderColor() const;
    void setBorderColor(const Vector4 &color);

    Vector4 corners() const;
    void setCorners(const Vector4 &corners);

    bool isCheckable() const;
    void setCheckable(bool checkable);

    bool isChecked() const;
    void setChecked(bool checked);

    bool isExclusive() const;
    void setExclusive(bool exclusive);

    void setHovered(bool hover, bool instant = false);

    void draw() override;

public: // signals
    void pressed();
    void clicked();

    void toggled(bool checked);

protected:
    void composeComponent() override;

    void updateBackgroundColor(const Vector4 &color);

    void update(const Vector2 &pos) override;

    void applyStyle() override;

    virtual void checkStateSet();

    void boundChanged(const Vector2 &) override;

protected:
    Vector4 m_borderRadius;
    Vector4 m_borderColor;
    Vector4 m_backgroundColor;
    Vector4 m_highlightedColor;
    Vector4 m_pressedColor;
    Vector4 m_currentColor;

    Sprite *m_backgroundImage;

    Mesh *m_backgroundMesh;

    MaterialInstance *m_imageMaterial;
    MaterialInstance *m_frameMaterial;

    float m_fadeDuration;
    float m_currentFade;

    bool m_dirtyBackground;

    bool m_hovered;
    bool m_checkable;
    bool m_checked;
    bool m_exclusive;

};

#endif // ABSTRACTBUTTON_H
