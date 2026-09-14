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
#ifndef SELECTORGROUP_H
#define SELECTORGROUP_H

#include "selector.h"

#include <vector>

class GroupSelector: public Selector {
public:
    GroupSelector();
    ~GroupSelector();

    void addSelector(Selector *);

    bool isMeet(Widget *widget) override;
    bool isBaseSelector() const override;
    int weight() override;

private:
    std::vector<Selector *> m_selectors;

    unsigned int targetSelector = 0;

};

#endif /* SELECTORGROUP_H */
