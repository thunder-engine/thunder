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

#include "resources/text.h"

#include <variant.h>

namespace {
    const char *gData("Data");
}

/*!
    \class Text
    \brief Text file resource.
    \inmodule Resources
*/

Text::Text() {

}

Text::~Text() {

}
/*!
    \internal
*/
void Text::loadUserData(const VariantMap &data) {
    auto it = data.find(gData);
    if(it != data.end()) {
        m_data = (*it).second.toByteArray();
    }
}
/*!
    \internal
*/
VariantMap Text::saveUserData() const {
    VariantMap result;
    result[gData] = m_data;
    return result;
}
/*!
    Returns text content as a raw byte array.
*/
uint8_t *Text::data() {
    return reinterpret_cast<uint8_t *>(m_data.data());
}
/*!
    Returns size of the text resource.
*/
uint32_t Text::size() const {
    return m_data.size();
}
/*!
    Sets the new \a size of the text resource.
*/
void Text::setSize(uint32_t size) {
    m_data.resize(size);
}
/*!
    Returns text content as string.
*/
TString Text::text() {
    return TString(reinterpret_cast<const char *>(m_data.data()));
}
