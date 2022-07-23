#ifndef SCENE_H
#define SCENE_H

#include <engine.h>

class Resource;
class Component;

class ENGINE_EXPORT Scene : public Object {
    A_REGISTER(Scene, Object, General)

    A_NOPROPERTIES()
    A_METHODS(
        A_METHOD(Object *, Scene::find)
    )

public:
    Scene();
    ~Scene();

    Resource *resource() const;
    void setResource(Resource *resource);

    bool isModified() const;
    void setModified(bool flag);

private:
    mutable Resource *m_resource;

    bool m_modified;

};

#endif // SCENE_H
