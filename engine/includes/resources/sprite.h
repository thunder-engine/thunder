#ifndef SPRITE_H
#define SPRITE_H

#include "texture.h"
#include "mesh.h"

class ENGINE_EXPORT Sprite : public Resource {
    A_OBJECT(Sprite, Resource, Resources)

    A_PROPERTIES(
        A_PROPERTY(float, pixelsPerUnit, Sprite::pixelsPerUnit, Sprite::setPixelsPerUnit),
        A_PROPERTY(Vector4, bounds, Sprite::bounds, Sprite::setBounds),
        A_PROPERTY(Vector4, border, Sprite::border, Sprite::setBorder),
        A_PROPERTY(Vector2, pivot, Sprite::pivot, Sprite::setPivot),
        A_PROPERTY(Texture *, texture, Sprite::texture, Sprite::setTexture),
        A_PROPERTY(int, mode, Sprite::mode, Sprite::setMode)
    )

public:
    enum Mode {
        Simple = 0,
        Sliced,
        Tiled,
        Complex
    };

public:
    Sprite();
    ~Sprite();

    Mesh *mesh() const;

    float pixelsPerUnit() const;
    void setPixelsPerUnit(float pixels);

    Vector4 bounds() const;
    void setBounds(const Vector4 &bounds);

    Vector4 border() const;
    void setBorder(const Vector4 &border);

    Vector2 pivot() const;
    void setPivot(const Vector2 &pivot);

    Texture *texture() const;
    void setTexture(Texture *texture);

    int mode() const;
    void setMode(int mode);

    Mesh *composeMesh(Mesh *mesh, Mode mode, Vector2 &size);

private:
    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

    void recalcMesh() const;

private:
    Vector4 m_bounds;

    Vector4 m_border;

    Vector2 m_pivot;

    Texture *m_texture;

    Mesh *m_mesh;

    float m_pixelsPerUnit;

    int m_mode;

    bool m_dirtyMesh;

};

#endif // SPRITE_H
