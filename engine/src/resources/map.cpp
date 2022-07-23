#include "resources/map.h"

#include "components/scene.h"

class MapPrivate {
public:
    MapPrivate() :
            m_scene(nullptr) {

    }

    ~MapPrivate() {
        delete m_scene;
    }

    Scene *m_scene;
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
    Returns a scene which can be added to the scene graph.
*/
Scene *Map::scene() const {
    if(p_ptr->m_scene == nullptr) {
        auto &children = getChildren();
        if(!children.empty()) {
            p_ptr->m_scene = dynamic_cast<Scene *>(children.front());
            p_ptr->m_scene->setResource(const_cast<Map *>(this));
        }
    }
    return p_ptr->m_scene;
}
/*!
    \internal
*/
void Map::setScene(Scene *scene) {
    p_ptr->m_scene = scene;
    if(p_ptr->m_scene) {
        p_ptr->m_scene->setResource(this);
    }
}
