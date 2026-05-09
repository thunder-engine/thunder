#ifndef SCENE_H
#define SCENE_H

#include <engine.h>

class Map;
class World;

class ENGINE_EXPORT Scene : public Object {
    A_OBJECT(Scene, Object, General)

    A_NOPROPERTIES()
    A_METHODS(
        A_METHOD(Object *, Scene::find),
        A_METHOD(World *, Scene::world),
        A_METHOD(void, Scene::addToGroup),
        A_METHOD(void, Scene::removeFromGroup)
    )

public:
    Scene();
    Scene(const Scene &origin);

    World *world() const;

    void addToGroup(Object *object, const TString &group);
    void addToGroupByHash(Object *object, uint32_t hash);

    void removeFromGroup(Object *object, const TString &group);
    void removeFromGroupByHash(Object *object, uint32_t hash);

    ObjectList &getObjectsInGroup(const TString &group);
    ObjectList &getObjectsInGroupByHash(uint32_t hash);

    Map *map() const;
    void setMap(Map *map);

    bool isModified() const;
    void setModified(bool flag);

private:
    std::mutex m_mutex;
    std::unordered_map<uint32_t, Object::ObjectList> m_groups;

    mutable Map *m_map;

    bool m_modified;

};

#endif // SCENE_H
