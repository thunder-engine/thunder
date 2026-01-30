#ifndef IMAGE_H
#define IMAGE_H

#include "widget.h"

#include <sprite.h>
#include <material.h>

class Mesh;
class Texture;
class MaterialInstance;

class UIKIT_EXPORT Image : public Widget {
    A_OBJECT(Image, Widget, Components/UI)

    A_PROPERTIES(
        A_PROPERTYEX(Material *, material, Image::material, Image::setMaterial, "editor=Asset"),
        A_PROPERTYEX(Sprite *, sprite, Image::sprite, Image::setSprite, "editor=Asset"),
        A_PROPERTYEX(Mode, drawMode, Image::drawMode, Image::setDrawMode, "enum=Mode"),
        A_PROPERTYEX(Vector4, color, Image::color, Image::setColor, "editor=Color")
    )
    A_NOMETHODS()
    A_ENUMS(
        A_ENUM(Mode,
            A_VALUE(Simple),
            A_VALUE(Sliced),
            A_VALUE(Tiled)
        )
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

    void setTexture(Texture *texture);

    Vector4 color() const;
    void setColor(const Vector4 &color);

    int drawMode() const;
    void setDrawMode(int mode);

protected:
    void draw(CommandBuffer &buffer) override;

    void boundChanged(const Vector2 &size) override;

    void makeDefaultMesh();

    static void spriteUpdated(int state, void *ptr);

protected:
    Vector4 m_color;

    Vector2 m_size;

    Mesh *m_mesh;

    Sprite *m_sprite;

    Texture *m_texture;

    MaterialInstance *m_material;

    int m_drawMode;

    bool m_dirtyMesh;

    bool m_dirtyMaterial;

};

#endif // IMAGE_H
