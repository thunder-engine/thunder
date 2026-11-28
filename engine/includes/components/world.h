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

#ifndef WORLD_H
#define WORLD_H

#include <engine.h>
#include <system.h>
#include <scene.h>

typedef bool (*RayCastCallback)(System *system, World *graph, const Ray &ray, float maxDistance, Ray::Hit *hit);

class ENGINE_EXPORT World : public Object {
    A_OBJECT(World, Object, General)

    A_PROPERTIES(
        A_PROPERTY(Scene *, activeScene, World::activeScene, World::setActiveScene),
        A_PROPERTY(Object *, gameController, World::gameController, World::setGameController)
    )
    A_METHODS(
        A_SIGNAL(World::sceneLoaded),
        A_SIGNAL(World::sceneUnloaded),
        A_SIGNAL(World::activeSceneChanged),
        A_SIGNAL(World::graphUpdated),
        A_METHOD(Scene *, World::createScene),
        A_METHOD(Scene *, World::loadScene),
        A_METHOD(void, World::unloadScene)
    )

public:
    World();

    bool isActive();
    void setActive(bool flag);

    Scene *createScene(const TString &name);

    Scene *loadScene(const TString &path, bool additive);
    void unloadScene(Scene *scene);
    void unloadAll();

    Scene *activeScene() const;
    void setActiveScene(Scene *scene);

    Object *gameController() const;
    void setGameController(Object *controller);

    bool rayCast(const Ray &ray, float maxDistance, Ray::Hit *hit);

    void setRayCastHandler(RayCastCallback callback, System *system);

    std::list<Scene *> &scenes();

public: // signals
    void sceneLoaded();
    void sceneUnloaded();

    void activeSceneChanged();

    void graphUpdated();

private:
    void addScene(Scene *scene);
    void addChild(Object *child, int32_t position = -1) override;

private:
    std::list<Scene *> m_scenes;

    RayCastCallback m_rayCastCallback;
    System *m_rayCastSystem;

    Scene *m_activeScene;
    Object *m_gameController;

    bool m_update;

};

#endif // WORLD_H
