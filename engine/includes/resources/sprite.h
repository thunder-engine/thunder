#ifndef SPRITE_H
#define SPRITE_H

#include "texture.h"
#include "mesh.h"

class Texture;
class AtlasNode;

class ENGINE_EXPORT Sprite : public Resource {
    A_REGISTER(Sprite, Resource, Resources)

    A_METHODS(
        A_METHOD(int, Sprite::addElement),
        A_METHOD(Texture *, Sprite::texture),
        A_METHOD(void, Sprite::pack)
    )

public:
    Sprite();
    ~Sprite();

    int addElement(Texture *texture);

    Mesh *mesh(int key) const;
    void setMesh(int key, Mesh *mesh);

    Texture *texture() const;
    void setTexture(Texture *texture);

    void pack(int padding);

protected:
    void clearAtlas();

private:
    void resize(int32_t width, int32_t height);

    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

private:
    typedef unordered_map<int, Mesh *> Meshes;
    typedef deque<Texture *> Textures;

    Meshes m_meshes;

    Texture *m_texture;

    Textures m_sources;

    AtlasNode *m_root;

};

#endif // SPRITE_H
