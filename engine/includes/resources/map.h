#ifndef MAP_H
#define MAP_H

#include "resource.h"

class MapPrivate;

class NEXT_LIBRARY_EXPORT Map : public Resource {
    A_REGISTER(Map, Resource, Resources)

    A_PROPERTIES(
        A_PROPERTY(Chunk *, chunk, Map::chunk, Map::setChunk)
    )
    A_NOMETHODS()

public:
    Map();
    ~Map();

    Chunk *chunk() const;
    void setChunk(Chunk *chunk);

private:
    MapPrivate *p_ptr;
};

#endif // MAP_H
