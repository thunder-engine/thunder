#include "components/renderable.h"

#include "systems/rendersystem.h"

/*!
    \class Renderable
    \brief Base class for every object which can be drawn on the screen.
    \inmodule Engine

    \note This class must be a superclass only and shouldn't be created manually.
*/

Renderable::Renderable() {

}

Renderable::~Renderable() {
    static_cast<RenderSystem *>(system())->removeRenderable(this);
}
/*!
    \internal
*/
void Renderable::draw(CommandBuffer &buffer, uint32_t layer) {
    A_UNUSED(buffer);
    A_UNUSED(layer);
}
/*!
    Returns a bound box of the renderable object.
*/
AABBox Renderable::bound() const {
    return AABBox();
}
/*!
    Returns the prority value used to sort renadarble components before drawing.
    Lower values are rendered first and higher are rendered last.
*/
int Renderable::priority() const {
    return 0;
}
/*!
    \internal
*/
void Renderable::setSystem(ObjectSystem *system) {
    Object::setSystem(system);

    RenderSystem *render = static_cast<RenderSystem *>(system);
    render->addRenderable(this);
}
