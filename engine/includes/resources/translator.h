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

#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include "resource.h"

class ENGINE_EXPORT Translator : public Resource {
    A_OBJECT(Translator, Resource, Resources)

    A_NOPROPERTIES()
    A_METHODS(
        A_METHOD(TString, Translator::translate),
        A_METHOD(void, Translator::setPair)
    )

public:
    Translator();
    ~Translator() override;

    TString translate(const TString &source) const;

    void setPair(const TString &source, const TString &translation);

private:
    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

private:
    std::unordered_map<TString, TString> m_table;

};

#endif // TRANSLATOR_H
