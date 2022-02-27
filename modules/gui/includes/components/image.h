#ifndef IMAGE_H
#define IMAGE_H

#include "widget.h"

class Material;
class Sprite;

class ImagePrivate;

class GUI_EXPORT Image : public Widget {
    A_REGISTER(Image, Widget, Components/UI)

    A_PROPERTIES(
        A_PROPERTYEX(Material *, material, Image::material, Image::setMaterial, "editor=Template"),
        A_PROPERTYEX(Sprite *, sprite, Image::sprite, Image::setSprite, "editor=Template"),
        A_PROPERTYEX(Vector4, color, Image::color, Image::setColor, "editor=Color"),
        A_PROPERTY(string, item, Image::item, Image::setItem)
    )
    A_NOMETHODS()

public:
    Image();
    ~Image();

    Material *material() const;
    void setMaterial(Material *material);

    Sprite *sprite() const;
    void setSprite(Sprite *sprite);

    Vector4 color() const;
    void setColor(const Vector4 &color);

    string item() const;
    void setItem(const string &item);

private:
    void draw(CommandBuffer &buffer, uint32_t layer) override;

    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

    void boundChanged() override;

    void composeComponent() override;

private:
    ImagePrivate *p_ptr;
};

#endif // IMAGE_H
