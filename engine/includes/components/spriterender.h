#ifndef SPRITERENDER
#define SPRITERENDER

#include "renderable.h"

class Sprite;
class Texture;
class Material;
class MaterialInstance;
class Mesh;

class ENGINE_EXPORT SpriteRender : public Renderable {
    A_OBJECT(SpriteRender, Renderable, Components/2D)

    A_PROPERTIES(
        A_PROPERTYEX(Material *, material, SpriteRender::material, SpriteRender::setMaterial, "editor=Asset"),
        A_PROPERTYEX(Sprite *, sprite, SpriteRender::sprite, SpriteRender::setSprite, "editor=Asset"),
        A_PROPERTYEX(Vector4, color, SpriteRender::color, SpriteRender::setColor, "editor=Color"),
        A_PROPERTY(TString, item, SpriteRender::item, SpriteRender::setItem),
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

    Sprite *sprite() const;
    void setSprite(Sprite *sprite);

    Texture *texture() const;
    void setTexture(Texture *texture);

    Vector4 color() const;
    void setColor(const Vector4 &color);

    TString item() const;
    void setItem(const TString &item);

    Vector2 size() const;
    void setSize(const Vector2 &size);

    int drawMode() const;
    void setDrawMode(int mode);

    int layer() const;
    void setLayer(int layer);

    void setMaterial(Material *material) override;

private:
    Mesh *meshToDraw(int instance) override;

    void setMaterialsList(const std::list<Material *> &materials) override;

    AABBox localBound() const override;

    void composeComponent() override;

    static void spriteUpdated(int state, void *ptr);

private:
    TString m_item;

    Vector4 m_color;

    Vector2 m_size;

    Sprite *m_sheet;

    Texture *m_texture;

    Mesh *m_mesh;
    Mesh *m_customMesh;

    int m_drawMode;

    int m_priority;

    bool m_useCustom;

    bool m_dirty;

};

#endif // SPRITERENDER

