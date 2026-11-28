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

#ifndef CONTROLSCHEME_H
#define CONTROLSCHEME_H

#include <resource.h>

class ENGINE_EXPORT ControlScheme : public Resource {
    A_OBJECT(ControlScheme, Resource, Resources)

public:
    int actionsCount() const;
    TString actionName(int action) const;

    int bindingsCount(int action) const;
    int bindingCode(int action, int binding);
    bool bindingNegative(int action, int binding);

protected:
    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

private:
    struct Binding {
        TString path;
        int32_t code;
        bool negative;
    };

    struct Action {
        TString name;
        std::vector<Binding> bindings;
    };

    std::vector<Action> m_actions;

};

#endif // CONTROLSCHEME_H
