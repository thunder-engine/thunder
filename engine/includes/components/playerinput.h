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

#ifndef PLAYERINPUT_H
#define PLAYERINPUT_H

#include <nativebehaviour.h>

#include <controlscheme.h>

class ENGINE_EXPORT PlayerInput : public NativeBehaviour {
    A_OBJECT(PlayerInput, NativeBehaviour, Components)

    A_PROPERTIES(
        A_PROPERTY(ControlScheme *, controlScheme, PlayerInput::controlScheme, PlayerInput::setControlScheme)
    )

public:
    PlayerInput();
    ~PlayerInput();

    float axis(const TString &name);
    bool button(const TString &name);

    ControlScheme *controlScheme() const;
    void setControlScheme(ControlScheme *scheme);

private:
    void update() override;

private:
    ControlScheme *m_controlScheme;

    std::unordered_map<TString, std::pair<int, float>> m_inputActions;

};

#endif // PLAYERINPUT_H
