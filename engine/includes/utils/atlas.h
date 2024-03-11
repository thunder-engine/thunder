#ifndef ATLAS_H
#define ATLAS_H

#include "engine.h"

class AtlasNode {
public:
    AtlasNode();
    ~AtlasNode();

    AtlasNode *insert(uint32_t width, uint32_t height);

    AtlasNode *left;
    AtlasNode *right;
    AtlasNode *parent;

    uint32_t x;
    uint32_t y;
    uint32_t w;
    uint32_t h;

    bool fill;
    bool dirty;

};

class Atlas {
public:
    void packSheets(int padding);

};

#endif // ATLAS_H
