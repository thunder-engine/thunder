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
