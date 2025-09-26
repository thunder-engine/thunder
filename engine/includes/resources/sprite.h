#ifndef SPRITE_H
#define SPRITE_H

#include "texture.h"
#include "mesh.h"

class Texture;

class ENGINE_EXPORT Sprite : public Resource {
    A_OBJECT(Sprite, Resource, Resources)

    A_PROPERTIES(
        A_PROPERTY(float, pixelsPerUnit, Sprite::pixelsPerUnit, Sprite::setPixelsPerUnit)
    )
    A_METHODS(
        A_METHOD(Texture *, Sprite::page)
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

    Mesh *shape(int key) const;
    void setShape(int key, Mesh *mesh);

    float pixelsPerUnit() const;
    void setPixelsPerUnit(float pixels);

    void setRegion(int key, const Vector4 &bounds);
    void setRegion(int key, const Vector4 &bounds, const Vector4 &border, const Vector2 &pivot);

    Texture *page(int key = -1);
    void addPage(Texture *texture);

    Mesh *composeMesh(Mesh *mesh, int key, Mode mode, Vector2 &size) const;

protected:
    int addElement(Texture *texture);

    void packSheets(int padding);

    virtual void clear();

private:
    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

private:
    struct Shape {
        int page;

        int mode;

        Mesh *mesh;

        Vector4 bounds;

        Vector4 border;

        Vector2 pivot;
    };

    std::unordered_map<int, Shape> m_shapes;

    std::vector<Texture *> m_pages;

    std::vector<Texture *> m_sources;

    float m_pixelsPerUnit;

};

#endif // SPRITE_H
