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
#ifndef SIGNSELECTOR_H
#define SIGNSELECTOR_H

#include "selector.h"

class SignSelector: public Selector {
public:
    enum SignType {
        NormalInherit,
        Plus,
        Greater,
        Tidle,
        Concat,
        Comma,
    };

    SignSelector(SignType type);

    SignType signType();

    bool operator >(SignSelector *);

    bool isMeet(Widget *) override;
    bool isBaseSelector() const override;
    int weight() override;

private:
    SignType m_signType;

};

#endif /* SIGNSELECTOR_H */
