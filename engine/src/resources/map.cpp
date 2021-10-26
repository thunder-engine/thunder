#include "resources/map.h"

#include "components/chunk.h"

class MapPrivate {
public:
    MapPrivate() :
            m_chunk(nullptr) {

    }

    Chunk *m_chunk;
};

/*!
    \class Map
    \brief A container which holds deserialized Scene object.
    \inmodule Resources
*/

Map::Map() :
        p_ptr(new MapPrivate) {

}

Map::~Map() {
    delete p_ptr;
}
/*!
    Returns a scene chunk which can be added to the scene.
*/
Chunk *Map::chunk() const {
    if(p_ptr->m_chunk == nullptr) {
        auto &children = getChildren();
        if(!children.empty()) {
            p_ptr->m_chunk = dynamic_cast<Chunk *>(children.front());
        }
    }
    return p_ptr->m_chunk;
}
/*!
    \internal
*/
void Map::setChunk(Chunk *chunk) {
    p_ptr->m_chunk = chunk;
    if(p_ptr->m_chunk) {
        p_ptr->m_chunk->setResource(this);
    }
}
