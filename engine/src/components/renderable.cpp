#include "components/renderable.h"

Renderable::Renderable() {

}

void Renderable::draw(ICommandBuffer &buffer, uint32_t layer) {
    A_UNUSED(buffer);
    A_UNUSED(layer);
}
/*!
    Returns a bound box of the renderable object.
*/
AABBox Renderable::bound() const {
    return AABBox();
}
