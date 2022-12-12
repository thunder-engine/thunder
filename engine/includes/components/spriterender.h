#ifndef SPRITERENDER
#define SPRITERENDER

#include "renderable.h"

class Sprite;
class Texture;
class Material;
class Mesh;
class Lod;
class SpriteRenderPrivate;

class ENGINE_EXPORT SpriteRender : public Renderable {
    A_REGISTER(SpriteRender, Renderable, Components/2D)

    A_PROPERTIES(
        A_PROPERTYEX(Sprite *, sprite, SpriteRender::sprite, SpriteRender::setSprite, "editor=Asset"),
        A_PROPERTYEX(Material *, material, SpriteRender::material, SpriteRender::setMaterial, "editor=Asset"),
        A_PROPERTYEX(Vector4, color, SpriteRender::color, SpriteRender::setColor, "editor=Color"),
        A_PROPERTY(string, item, SpriteRender::item, SpriteRender::setItem),
        A_PROPERTY(Vector2, size, SpriteRender::size, SpriteRender::setSize),
        A_PROPERTYEX(DrawMode, drawMode, SpriteRender::drawMode, SpriteRender::setDrawMode, "enum=DrawMode"),
        A_PROPERTY(int, layer, SpriteRender::layer, SpriteRender::setLayer)
    )
    A_NOMETHODS()
    A_ENUMS(
        A_ENUM(DrawMode,
               A_VALUE(Simple),
               A_VALUE(Sliced),
               A_VALUE(Tiled))
    )

public:
    enum DrawMode {
        Simple = 0,
        Sliced,
        Tiled
    };

public:
    SpriteRender();
    ~SpriteRender();

    Material *material() const;
    void setMaterial(Material *material);

    Sprite *sprite() const;
    void setSprite(Sprite *sprite);

    Texture *texture() const;
    void setTexture(Texture *texture);

    Vector4 &color() const;
    void setColor(const Vector4 color);

    string item() const;
    void setItem(const string item);

    Vector2 &size() const;
    void setSize(const Vector2 size);

    int drawMode() const;
    void setDrawMode(int mode);

    int layer() const;
    void setLayer(int layer);

    static bool composeMesh(Sprite *sprite, int key, Mesh *spriteMesh, Vector2 &size, int mode, bool resetSize, float scale = 1.0f);

private:
    void draw(CommandBuffer &buffer, uint32_t layer) override;

    AABBox localBound() const override;

    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

    void composeComponent() override;

    int priority() const override;

    static bool composeSliced(Mesh *mesh, Vector2 &size, Vector3 &delta, float scale);
    static bool composeTiled(Mesh *mesh, Vector2 &size, Vector3 &delta, float scale);

private:
    SpriteRenderPrivate *p_ptr;

};

#endif // SPRITERENDER

