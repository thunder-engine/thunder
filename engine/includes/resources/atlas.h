#ifndef ATLAS_H
#define ATLAS_H

#include "engine.h"

class NEXT_LIBRARY_EXPORT AtlasNode {
public:
    AtlasNode();
    ~AtlasNode();

    AtlasNode *insert(int32_t width, int32_t height);

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

#endif // ATLAS_H
