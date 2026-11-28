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

#ifndef TEXT_H
#define TEXT_H

#include "resource.h"

class ENGINE_EXPORT Text : public Resource {
    A_OBJECT(Text, Resource, Resources)

    A_PROPERTIES(
        A_PROPERTY(int, size, Text::size, Text::setSize)
    )

    A_METHODS(
        A_METHOD(TString, Text::text)
    )

public:
    Text();
    ~Text();

    uint32_t size() const;
    void setSize(uint32_t);

    TString text();

    uint8_t *data();

protected:
    void loadUserData(const VariantMap &data) override;

    VariantMap saveUserData() const override;

private:
    ByteArray m_data;

};

#endif // TEXT_H
