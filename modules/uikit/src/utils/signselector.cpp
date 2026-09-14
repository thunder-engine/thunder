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
#include "utils/signselector.h"

SignSelector::SignSelector(SignType type) {
    m_signType = type;
    m_selectorType = Selector::SignSelector;
}

bool SignSelector::operator>(SignSelector *other) {
    SignType otherType = other->signType();
    return m_signType == Concat && otherType != Concat;
}

SignSelector::SignType SignSelector::signType() {
    return m_signType;
}

bool SignSelector::isMeet(Widget *) {
    return false;
}

bool SignSelector::isBaseSelector() const {
    return true;
}

int SignSelector::weight() {
    return 0;
}
