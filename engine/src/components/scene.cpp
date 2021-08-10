#include "components/scene.h"

#include "components/actor.h"
#include "components/nativebehaviour.h"

namespace {
    static map<string, Variant> s_settingsMap;
};

PostProcessSettings::PostProcessSettings() {

}

map<string, Variant> &PostProcessSettings::settings() {
    return s_settingsMap;
}

void PostProcessSettings::registerSetting(const string &name, const Variant &value) {
    s_settingsMap[name + "_override"] = false;
    s_settingsMap[name] = value;
}

Variant PostProcessSettings::readValue(const string &name) const {
    auto it = m_currentValues.find(name);
    if(it != m_currentValues.end()) {
        return it->second;
    }
    return Variant();
}

void PostProcessSettings::writeValue(const string &name, const Variant &value) {
    m_currentValues[name] = value;
}

void PostProcessSettings::lerp(const PostProcessSettings &settings, float t) {
    for(auto &it : settings.m_currentValues) {
        size_t pos = it.first.find("_override");
        if(pos != std::string::npos) {
            if(it.second.toBool()) {
                const string key(it.first.substr(0, pos));
                const Variant value1 = m_currentValues[key];
                auto v = settings.m_currentValues.find(key);
                if(v != settings.m_currentValues.end()) {
                    Variant value2 = v->second;
                    switch(value1.type()) {
                        case MetaType::INTEGER: m_currentValues[key] = MIX(value1.toInt(), value2.toInt(), t); break;
                        case MetaType::FLOAT:   m_currentValues[key] = MIX(value1.toFloat(), value2.toFloat(), t); break;
                        case MetaType::VECTOR2: m_currentValues[key] = MIX(value1.toVector2(), value2.toVector2(), t); break;
                        case MetaType::VECTOR3: m_currentValues[key] = MIX(value1.toVector3(), value2.toVector3(), t); break;
                        case MetaType::VECTOR4: m_currentValues[key] = MIX(value1.toVector4(), value2.toVector4(), t); break;
                        default: break;
                    }
                }
            }
        }
    }
}

void PostProcessSettings::resetDefault() {
    m_currentValues.insert(s_settingsMap.begin(), s_settingsMap.end());
}

class ScenePrivate {
public:
    ScenePrivate() :
        m_Dirty(true),
        m_Update(false) {

    }

    bool m_Dirty;
    bool m_Update;

    PostProcessSettings m_FinalPostProcessSettings;
};

Scene::Scene() :
    p_ptr(new ScenePrivate) {

}

Scene::~Scene() {
    delete p_ptr;
}

bool Scene::isToBeUpdated() {
    return p_ptr->m_Update;
}
void Scene::setToBeUpdated(bool flag) {
    p_ptr->m_Update = flag;
}

bool Scene::isDirty() {
    return p_ptr->m_Dirty;
}
void Scene::setDirty(bool flag) {
    p_ptr->m_Dirty = flag;
}

PostProcessSettings &Scene::finalPostProcessSettings() {
    return p_ptr->m_FinalPostProcessSettings;
}
