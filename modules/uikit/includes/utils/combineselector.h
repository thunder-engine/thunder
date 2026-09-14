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
#ifndef COMBINESELECTOR_H
#define COMBINESELECTOR_H

#include "selector.h"

#include <vector>
#include <list>

class Widget;

class CombineSelector: public Selector {
public:
    enum CombineType {
        InstanceSibling,
        NormalSibling,
        InstanceInherical,
        NormalInherical,
        NoCombine
    };

public:
    CombineSelector();
    ~CombineSelector();

    void initialInstanceSiblingList(Selector *head, Selector *sibling);
    void initialNormalSiblingList(Selector *head, Selector *sibling);
    void initialInstanceInhericalList(Selector *root, Selector *child);
    void initialNormalInhericalList(Selector *root, Selector *child);

    bool isMeet(Widget *widget) override;
    bool isBaseSelector() const override;
    int weight() override;

    Selector *before();
    Selector *after();

    std::vector<Widget *> matchingWidgets;

private:
    std::list<Selector*> m_instanceSiblingList;
    std::list<Selector *> m_normalSiblingList;
    std::list<Selector *> m_instanceInhericalList;
    std::list<Selector *> m_normalInhericalList;

    CombineType m_combineType;

};

#endif /* COMBINESELECTOR_H */
