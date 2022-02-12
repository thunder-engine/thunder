#ifndef ABSTRACTBUTTON_H
#define ABSTRACTBUTTON_H

#include "widget.h"

class AbstractButtonPrivate;
class Image;

class GUI_EXPORT AbstractButton : public Widget {
    A_REGISTER(AbstractButton, Widget, General)

    A_PROPERTIES(
        A_PROPERTYEX(Image *, targetGraphic, AbstractButton::targetGraphic, AbstractButton::setTargetGraphic, "editor=Component"),
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

    float fadeDuration() const;
    void setFadeDuration(float duration);

    Vector4 highlightedColor() const;
    void setHighlightedColor(const Vector4 &color);

    Vector4 normalColor() const;
    void setNormalColor(const Vector4 &color);

    Vector4 pressedColor() const;
    void setPressedColor(const Vector4 &color);

    Image *targetGraphic() const;
    void setTargetGraphic(Image *image);

    void clicked();

protected:
    void onReferenceDestroyed() override;

    void composeComponent() override;

    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

    void update() override;

    virtual void onClicked();

private:
    AbstractButtonPrivate *p_ptr;

};

#endif // ABSTRACTBUTTON_H
