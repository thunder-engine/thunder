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
#include "utils/classselector.h"

#include "components/widget.h"

#include <algorithm>

ClassSelector::ClassSelector(const TString &cls) {
    m_class = cls;
    m_selectorType = Selector::ClassSelector;
}

bool ClassSelector::isMeet(Widget *widget) {
    auto classes = widget->classes();
    return std::find(classes.begin(), classes.end(), m_class) != classes.end();
}

bool ClassSelector::isBaseSelector() const {
    return true;
}

int ClassSelector::weight() {
    return 10;
}
