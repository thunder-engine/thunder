#ifndef SPRITE_H
#define SPRITE_H

#include "texture.h"
#include "mesh.h"

class Texture;

class ENGINE_EXPORT Sprite : public Resource {
    A_REGISTER(Sprite, Resource, Resources)

    A_METHODS(
        A_METHOD(Texture *, Sprite::page)
    )

public:
    Sprite();
    ~Sprite();

    Mesh *shape(int key) const;
    void setShape(int key, Mesh *mesh);

    Texture *page(int key = -1) const;
    void addPage(Texture *texture);

protected:
    int addElement(Texture *texture);

    void packSheets(int padding);

    virtual void clear();

private:
    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

private:
    unordered_map<int, pair<Mesh *, uint32_t>> m_shapes;

    vector<Texture *> m_pages;

    vector<Texture *> m_sources;

};

#endif // SPRITE_H
