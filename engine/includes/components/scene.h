#ifndef SCENE_H
#define SCENE_H

#include "engine.h"

class ScenePrivate;

class PostProcessSettings;

class NEXT_LIBRARY_EXPORT Scene : public Object {
    A_REGISTER(Scene, Object, General)

    A_NOPROPERTIES()
    A_NOMETHODS()

public:
    Scene();
    ~Scene();

    bool isToBeUpdated();
    void setToBeUpdated(bool flag);

    bool isDirty();
    void setDirty(bool flag = true);

    PostProcessSettings &finalPostProcessSettings();

private:
    ScenePrivate *p_ptr;

};

#endif // SCENE_H
