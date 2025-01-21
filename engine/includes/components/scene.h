#ifndef SCENE_H
#define SCENE_H

#include <engine.h>

class Map;
class World;

class ENGINE_EXPORT Scene : public Object {
    A_REGISTER(Scene, Object, General)

    A_NOPROPERTIES()
    A_METHODS(
        A_METHOD(Object *, Scene::find),
        A_METHOD(World *, Scene::world)
    )

public:
    Scene();

    World *world() const;

    Map *map() const;
    void setMap(Map *map);

    bool isModified() const;
    void setModified(bool flag);

private:
    mutable Map *m_map;

    bool m_modified;

};

#endif // SCENE_H
