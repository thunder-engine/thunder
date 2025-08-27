#ifndef SPRITE_H
#define SPRITE_H

#include "texture.h"
#include "mesh.h"

class Texture;

class ENGINE_EXPORT Sprite : public Resource {
    A_OBJECT(Sprite, Resource, Resources)

    A_METHODS(
        A_METHOD(Texture *, Sprite::page)
    )

public:
    enum Mode {
        Simple = 0,
        Sliced,
        Tiled
    };

public:
    Sprite();
    ~Sprite();

    Mesh *shape(int key) const;
    void setShape(int key, Mesh *mesh);

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
    std::unordered_map<int, std::pair<Mesh *, uint32_t>> m_shapes;

    std::vector<Texture *> m_pages;

    std::vector<Texture *> m_sources;

};

#endif // SPRITE_H
