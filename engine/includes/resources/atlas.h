#ifndef ATLAS_H
#define ATLAS_H

#include "texture.h"

class Texture;
class AtlasPrivate;

class AtlasNode {
public:
    AtlasNode();
    ~AtlasNode();

    AtlasNode *insert(int width, int height);

    bool clean();

    bool fill;
    bool dirty;

    int x;
    int y;
    int w;
    int h;

    AtlasNode *parent;
    AtlasNode *child[2];
};

class NEXT_LIBRARY_EXPORT Atlas : public Resource {
    A_REGISTER(Atlas, Resource, Resources)

    A_METHODS(
        A_METHOD(int, Atlas::addElement),
        A_METHOD(Vector4, Atlas::uv),
        A_METHOD(Texture *, Atlas::texture),
        A_METHOD(void, Atlas::pack)
    )

public:
    Atlas();
    ~Atlas();

    int addElement(Texture *texture);

    Vector2Vector shape(int index) const;

    Vector4 uv(int index) const;

    Texture *texture() const;

    void pack(int padding);

protected:
    void clearAtlas();

private:
    void resize(int32_t width, int32_t height);

    AtlasPrivate *p_ptr;

};

#endif // ATLAS_H
