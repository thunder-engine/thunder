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
#include "utils/pseudoselector.h"

PseudoSelector::PseudoSelector(const TString &data) {
    m_selectorType = Selector::PseudoSelector;
    m_data = data;
    m_parameter = NULL;
}

PseudoSelector::~PseudoSelector() {
    delete m_parameter;
    m_parameter = NULL;
}

bool PseudoSelector::isMeet(Widget *) {
    return false;
}

bool PseudoSelector::isBaseSelector() const {
    return true;
}

int PseudoSelector::weight() {
    return 10;
}

void PseudoSelector::setParameter(PseudoSelector::Parameter *p) {
    if(m_parameter == p) {
        return;
    }

    if(m_parameter) {
        delete m_parameter;
    }
    m_parameter = p;
}

PseudoSelector::Parameter* PseudoSelector::parameter() {
    return m_parameter;
}
