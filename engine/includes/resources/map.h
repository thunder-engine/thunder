#ifndef MAP_H
#define MAP_H

#include "resource.h"

class MapPrivate;

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
    MapPrivate *p_ptr;
};

#endif // MAP_H
