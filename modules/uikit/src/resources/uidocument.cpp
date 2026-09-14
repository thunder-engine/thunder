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
#include "resources/uidocument.h"

namespace {
    const char *gData = "Data";
}

/*!
    \class UiDocument
    \brief UiDocument class, which appears to handle the storage and management of UI data.
    \inmodule Gui
*/

UiDocument::UiDocument() {

}

/*!
    Returns content as a string.
*/
TString UiDocument::data() const {
    return m_data;
}
/*!
    Sets a new content \a data.
*/
void UiDocument::setData(const TString &data) {
    m_data = data;
}
/*!
    \internal
*/
void UiDocument::loadUserData(const VariantMap &data) {
    auto it = data.find(gData);
    if(it != data.end()) {
        m_data = (*it).second.toString();
    }
}
/*!
    \internal
*/
VariantMap UiDocument::saveUserData() const {
    VariantMap result;
    result[gData] = m_data;
    return result;
}
