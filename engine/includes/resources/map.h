#ifndef MAP_H
#define MAP_H

#include <resource.h>

#include <scene.h>

class ENGINE_EXPORT Map : public Resource {
    A_REGISTER(Map, Resource, Resources)

    A_PROPERTIES(
        A_PROPERTY(Scene *, scene, Map::scene, Map::setScene)
    )
    A_NOMETHODS()

public:
    Map();
    ~Map();

    Scene *scene() const;
    void setScene(Scene *scene);

private:
    mutable Scene *m_scene;

};

#endif // MAP_H
