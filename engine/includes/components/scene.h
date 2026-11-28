/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

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
    ~Scene();

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
