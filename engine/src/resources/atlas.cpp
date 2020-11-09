#include "resources/atlas.h"

AtlasNode::AtlasNode() :
        fill(false),
        dirty(false),
        x(0),
        y(0),
        w(1),
        h(1),
        parent(nullptr) {
    child[0] = nullptr;
    child[1] = nullptr;
}

AtlasNode::~AtlasNode() {
    if(parent) {
        if(parent->child[0] == this) {
            parent->child[0] = nullptr;
        } else {
            parent->child[1] = nullptr;
        }
    }
    delete child[0];
    delete child[1];
}

AtlasNode *AtlasNode::insert(int32_t width, int32_t height) {
    PROFILE_FUNCTION();

    if(child[0]) {
        AtlasNode *node = child[0]->insert(width, height);
        if(node) {
            return node;
        }
        return child[1]->insert(width, height);
    }

    if(fill || w < width || h < height) {
        return nullptr;
    }

    if(w == width && h == height) {
        return this;
    }
    // Request is smaller then node start splitting
    int32_t sw = w - width;
    int32_t sh = h - height;

    child[0] = new AtlasNode;
    child[0]->parent = this;
    child[1] = new AtlasNode;
    child[1]->parent = this;

    if(sw > sh) { // Horizontal
        child[0]->x = x;
        child[0]->y = y;
        child[0]->w = width;
        child[0]->h = h;

        child[1]->x = x + width;
        child[1]->y = y;
        child[1]->w = sw;
        child[1]->h = h;
    } else { // Vertical
        child[0]->x = x;
        child[0]->y = y;
        child[0]->w = w;
        child[0]->h = height;

        child[1]->x = x;
        child[1]->y = y + height;
        child[1]->w = w;
        child[1]->h = sh;
    }

    return child[0]->insert(width, height);
}

bool AtlasNode::clean () {
    PROFILE_FUNCTION();

    if(child[0] && child[0]->clean()) {
        child[0] = nullptr;
        delete child[1];
        child[1] = nullptr;
    }

    if(child[0] == nullptr && child[1] == nullptr && !fill && parent) {
        delete this;
        return true;
    }
    return false;
}
