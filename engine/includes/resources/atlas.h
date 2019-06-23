#ifndef ATLAS_H
#define ATLAS_H

#include "resource.h"

class Texture;
class AtlasPrivate;

class NEXT_LIBRARY_EXPORT Atlas : public Resource {
    A_REGISTER(Atlas, Resource, Resources)

public:
    Atlas ();
    ~Atlas ();

    uint32_t addElement (const Texture *texture);

    Vector2Vector shape (uint32_t index) const;

    Vector4 uv (uint32_t index) const;

    Texture *texture () const;

    void clear ();

    void pack (uint8_t padding);

private:
    AtlasPrivate *p_ptr;

};

#endif // ATLAS_H
