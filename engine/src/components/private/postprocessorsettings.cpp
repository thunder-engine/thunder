#include "components/private/postprocessorsettings.h"

std::map<std::string, Variant> PostProcessSettings::s_settingsMap;

PostProcessSettings::PostProcessSettings() {

}

std::map<std::string, Variant> &PostProcessSettings::settings() {
    return s_settingsMap;
}

void PostProcessSettings::registerSetting(const std::string &name, const Variant &value) {
    s_settingsMap[name] = value;
}

Variant PostProcessSettings::defaultValue(const std::string &name) {
    auto it = s_settingsMap.find(name);
    if(it != s_settingsMap.end()) {
        return it->second;
    }

    return Variant();
}

Variant PostProcessSettings::readValue(const std::string &name) const {
    auto it = m_currentValues.find(name);
    if(it != m_currentValues.end()) {
        return it->second;
    }

    return Variant();
}

void PostProcessSettings::writeValue(const std::string &name, const Variant &value) {
    m_currentValues[name] = value;
}

void PostProcessSettings::resetDefault() {
    m_currentValues.clear();
}
