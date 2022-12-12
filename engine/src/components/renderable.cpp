#include "components/renderable.h"

#include "components/transform.h"

#include "systems/rendersystem.h"

/*!
    \class Renderable
    \brief Base class for every object which can be drawn on the screen.
    \inmodule Engine

    \note This class must be a superclass only and shouldn't be created manually.
*/

Renderable::Renderable() :
    m_transformHash(0) {

}

Renderable::~Renderable() {
    static_cast<RenderSystem *>(system())->removeRenderable(this);
}
/*!
    Returns a bound box of the renderable object.
*/
AABBox Renderable::bound() const {
    AABBox bb = localBound();
    Transform *t = transform();
    int32_t hash = t->hash();
    if(hash != m_transformHash || m_localBox != bb) {
        m_localBox = bb;
        m_worldBox = m_localBox * t->worldTransform();
        m_transformHash = hash;
    }
    return m_worldBox;
}
/*!
    \internal
*/
void Renderable::draw(CommandBuffer &buffer, uint32_t layer) {
    A_UNUSED(buffer);
    A_UNUSED(layer);
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
AABBox Renderable::localBound() const {
    return AABBox();
}
/*!
    \internal
*/
void Renderable::setSystem(ObjectSystem *system) {
    Object::setSystem(system);

    RenderSystem *render = static_cast<RenderSystem *>(system);
    render->addRenderable(this);
}
