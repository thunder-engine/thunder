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

#include "resources/translator.h"

#include <metaproperty.h>

namespace  {
    const char *gData = "Data";
}

/*!
    \class Translator
    \brief Translator resource provides a translation table.
    \inmodule Resources
*/

Translator::Translator() {

}

Translator::~Translator() {

}
/*!
    Returns the translated \a source string.
*/
TString Translator::translate(const TString &source) const {
    auto it = m_table.find(source);
    if(it != m_table.end()) {
        return it->second;
    }
    return source;
}
/*!
    Sets new \a translation for the \a source string.
*/
void Translator::setPair(const TString &source, const TString &translation) {
    m_table[source] = translation;
}
/*!
    \internal
*/
void Translator::loadUserData(const VariantMap &data) {
    auto it = data.find(gData);
    if(it != data.end()) {
        for(auto &pair : (*it).second.toMap()) {
            m_table[pair.first] = pair.second.toString();
        }
    }
}
/*!
    \internal
*/
VariantMap Translator::saveUserData() const {
    VariantMap result;
    VariantMap data;

    for(auto &it : m_table) {
        data[it.first] = it.second;
    }

    result[gData] = data;
    return result;
}

