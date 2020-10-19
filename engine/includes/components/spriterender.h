#ifndef SPRITERENDER
#define SPRITERENDER

#include "renderable.h"

class Texture;
class Material;
class Mesh;
class SpriteRenderPrivate;

class NEXT_LIBRARY_EXPORT SpriteRender : public Renderable {
    A_REGISTER(SpriteRender, Renderable, Components)

    A_PROPERTIES(
        A_PROPERTYEX(Material *, material, SpriteRender::material, SpriteRender::setMaterial, "editor=Template"),
        A_PROPERTYEX(Texture *, texture, SpriteRender::texture, SpriteRender::setTexture, "editor=Template"),
        A_PROPERTYEX(Vector4, color, SpriteRender::color, SpriteRender::setColor, "editor=Color")
    )
    A_NOMETHODS()

public:
    SpriteRender ();
    ~SpriteRender ();

    Material *material () const;
    void setMaterial (Material *material);

    Texture *texture () const;
    void setTexture (Texture *texture);

    Vector4 color () const;
    void setColor (const Vector4 &color);

private:
    void draw (ICommandBuffer &buffer, uint32_t layer) override;

    AABBox bound () const override;

    void loadUserData (const VariantMap &data) override;
    VariantMap saveUserData () const override;

private:
    SpriteRenderPrivate *p_ptr;

};

#endif // SPRITERENDER

