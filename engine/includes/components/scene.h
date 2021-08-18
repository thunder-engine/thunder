#ifndef SCENE_H
#define SCENE_H

#include "engine.h"

class ScenePrivate;

class NEXT_LIBRARY_EXPORT PostProcessSettings {
public:
    PostProcessSettings();

    static map<string, Variant> &settings();

    static void registerSetting(const string &name, const Variant &value);

    Variant readValue(const string &name) const;
    void writeValue(const string &name, const Variant &value);

    void lerp(const PostProcessSettings &settings, float t);

    void resetDefault();

private:
    unordered_map<string, Variant> m_currentValues;

};

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
