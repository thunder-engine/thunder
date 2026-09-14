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
#include "utils/typeselector.h"

#include "components/widget.h"

TypeSelector::TypeSelector(const TString &typeName) {
    m_typeName = typeName;
    m_selectorType = Selector::TypeSelector;
}

TString TypeSelector::tagName() {
    return m_typeName;
}

bool TypeSelector::isMeet(Widget *widget) {
    return widget->typeName() == m_typeName;
}

bool TypeSelector::isBaseSelector() const {
    return true;
}

int TypeSelector::weight() {
    return 1;
}
