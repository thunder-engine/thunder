#include "components/renderable.h"

class RenderablePrivate {
public:
    bool m_WorldBoxDirty;

    AABBox m_WorldBox;



};

Renderable::Renderable() {

}

void Renderable::draw(ICommandBuffer &buffer, uint32_t layer) {
    A_UNUSED(buffer)
    A_UNUSED(layer)
}

AABBox Renderable::bound() const {
    return AABBox();
}
