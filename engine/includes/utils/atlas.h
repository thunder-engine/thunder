#ifndef ATLAS_H
#define ATLAS_H

#include <cstdint>

class AtlasNode {
public:
    AtlasNode();
    ~AtlasNode();

    AtlasNode *insert(uint32_t width, uint32_t height);

    bool clean();

    AtlasNode *left;
    AtlasNode *right;
    AtlasNode *parent;

    uint32_t x;
    uint32_t y;
    uint32_t w;
    uint32_t h;

    bool occupied;
};

#endif // ATLAS_H
