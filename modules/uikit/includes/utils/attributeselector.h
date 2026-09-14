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
#ifndef ATTRIBUTESELECTOR_H
#define ATTRIBUTESELECTOR_H

#include "selector.h"

class AttributeSelector: public Selector {
public:
    enum AttributeFilterRule {
        Prefix,
        Suffix,
        Include,
        Equal,
        Substring,
        DashMatch,
        NoRule
    };

public:
    AttributeSelector(const TString &key, const TString &value, AttributeFilterRule rule);

    bool isMeet(Widget *widget) override;
    bool isBaseSelector() const override;
    int weight() override;

private:
    TString m_key;
    TString m_value;

    AttributeFilterRule m_filterRule;

};

#endif /* ATTRIBUTESELECTOR_H */
