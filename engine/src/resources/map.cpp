#include "resources/map.h"

#include "components/scene.h"

/*!
    \class Map
    \brief A container which holds deserialized Scene object.
    \inmodule Resources
*/

Map::Map() :
        m_scene(nullptr) {

}

Map::~Map() {
    delete m_scene;
}
/*!
    Returns a scene which can be added to the scene graph.
*/
Scene *Map::scene() const {
    if(m_scene == nullptr) {
        auto &children = getChildren();
        if(!children.empty()) {
            m_scene = dynamic_cast<Scene *>(children.front());
            m_scene->setResource(const_cast<Map *>(this));
        }
    }
    return m_scene;
}
/*!
    \internal
*/
void Map::setScene(Scene *scene) {
    m_scene = scene;
    if(m_scene) {
        m_scene->setResource(this);
    }
}
