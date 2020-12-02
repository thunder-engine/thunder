#ifndef SPRITERENDER
#define SPRITERENDER

#include "renderable.h"

class Sprite;
class Texture;
class Material;
class Mesh;
class SpriteRenderPrivate;

class NEXT_LIBRARY_EXPORT SpriteRender : public Renderable {
    A_REGISTER(SpriteRender, Renderable, Components)

    A_PROPERTIES(
        A_PROPERTYEX(Material *, material, SpriteRender::material, SpriteRender::setMaterial, "editor=Template"),
        A_PROPERTYEX(Sprite *, sprite, SpriteRender::sprite, SpriteRender::setSprite, "editor=Template"),
        A_PROPERTYEX(Vector4, color, SpriteRender::color, SpriteRender::setColor, "editor=Color"),
        A_PROPERTY(string, item, SpriteRender::item, SpriteRender::setItem),
        A_PROPERTY(Vector2, size, SpriteRender::size, SpriteRender::setSize)
    )
    A_NOMETHODS()

public:
    SpriteRender();
    ~SpriteRender();

    Material *material() const;
    void setMaterial(Material *material);

    Sprite *sprite() const;
    void setSprite(Sprite *sprite);

    Texture *texture() const;
    void setTexture(Texture *texture);

    Vector4 color() const;
    void setColor(const Vector4 &color);

    string item() const;
    void setItem(const string &item);

    Vector2 size() const;
    void setSize(Vector2 &size);

private:
    void draw(ICommandBuffer &buffer, uint32_t layer) override;

    AABBox bound() const override;

    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

private:
    SpriteRenderPrivate *p_ptr;

};

#endif // SPRITERENDER

