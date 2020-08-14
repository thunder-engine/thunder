#ifndef ATLAS_H
#define ATLAS_H

#include "resource.h"

class Texture;
class AtlasPrivate;

class PackNode {
public:
    PackNode ();
    ~PackNode ();

    PackNode *insert (int32_t width, int32_t height);

    bool clean ();

    bool fill;
    bool dirty;

    int32_t x;
    int32_t y;
    int32_t w;
    int32_t h;

    PackNode *parent;
    PackNode *child[2];
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
