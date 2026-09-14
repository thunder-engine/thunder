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
#include "utils/selectorgroup.h"

#include <algorithm>

GroupSelector::GroupSelector() {
    m_selectorType = Selector::SelectorGroup;
}

GroupSelector::~GroupSelector() {
    for(auto it : m_selectors) {
        delete it;
    }
    m_selectors.clear();
}

void GroupSelector::addSelector(Selector *s) {
    if(!s) {
        return;
    }

    m_selectors.push_back(s);
}

bool GroupSelector::isMeet(Widget *widget) {
    for(const auto &s : m_selectors) {
        if(s->isMeet(widget)) {
            targetSelector = (std::find(m_selectors.begin(), m_selectors.end(), s) - m_selectors.begin());

            return true;
        }
    }
    return false;
}
    
bool GroupSelector::isBaseSelector() const {
    return false;
}

int GroupSelector::weight() {
    if(m_selectors.size() < targetSelector + 1) {
        return 0;
    }

    return m_selectors[targetSelector]->weight();
}
