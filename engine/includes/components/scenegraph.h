#ifndef SCENEGRAPH_H
#define SCENEGRAPH_H

#include "engine.h"

class SceneGraphPrivate;

class PostProcessSettings;

class ENGINE_EXPORT SceneGraph : public Object {
    A_REGISTER(SceneGraph, Object, General)

    A_NOPROPERTIES()
    A_METHODS(
        A_SIGNAL(SceneGraph::sceneLoaded),
        A_SIGNAL(SceneGraph::sceneUnloaded),
        A_SIGNAL(SceneGraph::activeSceneChanged)
    )

public:
    SceneGraph();
    ~SceneGraph();

    bool isToBeUpdated();
    void setToBeUpdated(bool flag);

    Scene *createScene(const string &name);

    Scene *loadScene(const string &path, bool additive);
    void unloadScene(Scene *scene);

    Scene *activeScene() const;
    void setActiveScene(Scene *scene);

    PostProcessSettings &finalPostProcessSettings();

public:
    void sceneLoaded();
    void sceneUnloaded();

    void activeSceneChanged();

private:
    void addChild(Object *child, int32_t position = -1) override;

private:
    SceneGraphPrivate *p_ptr;

};

#endif // SCENEGRAPH_H
