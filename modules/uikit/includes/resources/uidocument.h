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
#ifndef UIDOCUMENT_H
#define UIDOCUMENT_H

#include <resource.h>
#include <uikit.h>

class UIKIT_EXPORT UiDocument : public Resource {
    A_OBJECT(UiDocument, Resource, Resources)

public:
    UiDocument();

    TString data() const;
    void setData(const TString &data);

private:
    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

private:
    TString m_data;

};

#endif // UIDOCUMENT_H
