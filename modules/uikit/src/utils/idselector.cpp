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
#include "utils/idselector.h"

#include "components/widget.h"

IdSelector::IdSelector(const TString &id) {
    m_id = id;
    m_selectorType = Selector::IDSelector;
}

bool IdSelector::isMeet(Widget *widget) {
    return m_id == widget->name();
}

bool IdSelector::isBaseSelector() const {
    return true;
}

int IdSelector::weight() {
    return 100;
}
