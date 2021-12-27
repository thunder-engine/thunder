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
/*!
    Returns the component with \a type in the Chunk's children using depth search.
    A component is returned only if it's found on a current Chunk; otherwise returns nullptr.
*/
Component *Chunk::componentInChild(const string type) {
    PROFILE_FUNCTION();
    for(auto it : getChildren()) {
        Actor *actor = dynamic_cast<Actor *>(it);
        if(actor) {
            Component *result = actor->componentInChild(type);
            if(result) {
                return static_cast<Component *>(result);
            }
        }
    }
    return nullptr;
}
