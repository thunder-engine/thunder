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
#ifndef SWITCH_H
#define SWITCH_H

#include "checkbox.h"

class UIKIT_EXPORT Switch : public CheckBox {
    A_OBJECT(Switch, AbstractButton, Components/UI)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    Switch();

protected:
    void update(const Vector2 &pos) override;

    void checkStateSet() override;

    void composeComponent() override;

    void boundChanged(const Vector2 &size) override;

private:
    float m_switchDuration;
    float m_currentFade;

};

#endif // SWITCH_H
