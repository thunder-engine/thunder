#ifndef WORLD_H
#define WORLD_H

#include "engine.h"
#include "system.h"
#include "scene.h"

typedef bool (*RayCastCallback)(System *system, World *graph, const Ray &ray, float maxDistance, Ray::Hit *hit);

class ENGINE_EXPORT World : public Object {
    A_REGISTER(World, Object, General)

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

    bool isToBeUpdated();
    void setToBeUpdated(bool flag);

    void makeDirty();

    Scene *createScene(const std::string &name);

    Scene *loadScene(const std::string &path, bool additive);
    void unloadScene(Scene *scene);
    void unloadAll();

    Scene *activeScene() const;
    void setActiveScene(Scene *scene);

    Object *gameController() const;
    void setGameController(Object *controller);

    bool rayCast(const Ray &ray, float maxDistance, Ray::Hit *hit);

    void setRayCastHandler(RayCastCallback callback, System *system);

public: // signals
    void sceneLoaded();
    void sceneUnloaded();

    void activeSceneChanged();

    void graphUpdated();

private:
    void addChild(Object *child, int32_t position = -1) override;

private:
    RayCastCallback m_rayCastCallback;
    System *m_rayCastSystem;

    Scene *m_activeScene;
    Object *m_gameController;

    bool m_dirty;
    bool m_update;

};

#endif // WORLD_H
