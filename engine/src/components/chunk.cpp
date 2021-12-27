#include "components/chunk.h"

#include "components/actor.h"

Chunk::Chunk() :
    m_resource(nullptr) {

}

Chunk::~Chunk() {
    m_resource = nullptr;
}
/*!
    \internal
*/
Resource *Chunk::resource() const {
    return m_resource;
}
/*!
    \internal
*/
void Chunk::setResource(Resource *resource) {
    m_resource = resource;
}
