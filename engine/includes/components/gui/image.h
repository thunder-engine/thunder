#ifndef IMAGE_H
#define IMAGE_H

#include "widget.h"

class Material;
class MaterialInstance;
class Mesh;
class Sprite;

class ImagePrivate;

class ENGINE_EXPORT Image : public Widget {
    A_REGISTER(Image, Widget, Components/UI)

    A_PROPERTIES(
        A_PROPERTYEX(Material *, material, Image::material, Image::setMaterial, "editor=Template"),
        A_PROPERTYEX(Sprite *, sprite, Image::sprite, Image::setSprite, "editor=Template"),
        A_PROPERTY(string, item, Image::item, Image::setItem),
        A_PROPERTYEX(Mode, drawMode, Image::drawMode, Image::setDrawMode, "enum=Mode"),
        A_PROPERTYEX(Vector4, color, Image::color, Image::setColor, "editor=Color")
    )
    A_NOMETHODS()
    A_ENUMS(
        A_ENUM(Mode,
               A_VALUE(Simple),
               A_VALUE(Sliced),
               A_VALUE(Tiled))
    )

public:
    enum Mode {
        Simple = 0,
        Sliced,
        Tiled
    };

public:
    Image();
    ~Image();

    Material *material() const;
    virtual void setMaterial(Material *material);

    Sprite *sprite() const;
    void setSprite(Sprite *sprite);

    Vector4 color() const;
    void setColor(const Vector4 color);

    string item() const;
    void setItem(const string item);

    int drawMode() const;
    void setDrawMode(int mode);

protected:
    void draw(CommandBuffer &buffer, uint32_t layer) override;

    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

    void boundChanged(const Vector2 &size) override;

    void composeMesh();

protected:
    Mesh *m_mesh;

    MaterialInstance *m_material;
    MaterialInstance *m_customMaterial;

    Sprite *m_sprite;

    int m_hash;

    int m_drawMode;

    Vector4 m_color;

    Vector2 m_meshSize;

    string m_item;

private:
    friend class ImagePrivate;
    ImagePrivate *p_ptr;

};

#endif // IMAGE_H
