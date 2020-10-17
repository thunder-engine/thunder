#ifndef ATLAS_H
#define ATLAS_H

#include "resource.h"

class Texture;
class AtlasPrivate;

class AtlasNode {
public:
    AtlasNode ();
    ~AtlasNode ();

    AtlasNode *insert (int width, int height);

    bool clean ();

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

public:
    Atlas ();
    ~Atlas ();

    uint32_t addElement (Texture *texture);

    Vector2Vector shape (uint32_t index) const;

    Vector4 uv (uint32_t index) const;

    Texture *texture () const;

    void pack (uint8_t padding);

protected:
    void clearAtlas ();

private:
    void resize (int32_t width, int32_t height);

    AtlasPrivate *p_ptr;

};

#endif // ATLAS_H
