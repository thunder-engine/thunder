#include "utils/atlas.h"

AtlasNode::AtlasNode() :
        left(nullptr),
        right(nullptr),
        parent(nullptr),
        x(0),
        y(0),
        w(1),
        h(1),
        fill(false),
        dirty(false) {

}

AtlasNode::~AtlasNode() {
    if(parent) {
        if(parent->left == this) {
            parent->left = nullptr;
        } else {
            parent->right = nullptr;
        }
    }
    delete left;
    delete right;
}

AtlasNode *AtlasNode::insert(uint32_t width, uint32_t height) {
    PROFILE_FUNCTION();

    if(left) {
        AtlasNode *node = left->insert(width, height);
        if(node) {
            return node;
        }
        return right->insert(width, height);
    }

    if(fill || w < width || h < height) {
        return nullptr;
    }

    if(w == width && h == height) {
        return this;
    }
    // Request is smaller then node start splitting
    uint32_t sw = w - width;
    uint32_t sh = h - height;

    left = new AtlasNode;
    left->parent = this;
    right = new AtlasNode;
    right->parent = this;

    if(sw > sh) { // Horizontal
        left->x = x;
        left->y = y;
        left->w = width;
        left->h = h;

        right->x = x + width;
        right->y = y;
        right->w = sw;
        right->h = h;
    } else { // Vertical
        left->x = x;
        left->y = y;
        left->w = w;
        left->h = height;

        right->x = x;
        right->y = y + height;
        right->w = w;
        right->h = sh;
    }

    return left->insert(width, height);
}
