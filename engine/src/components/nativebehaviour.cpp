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

#include "components/nativebehaviour.h"

/*!
    \class NativeBehaviour
    \brief Base class for all programmable game logic.
    \inmodule Components

    All programmable game logic must be derived from NativeBehaviour class.

    Example:
    \code
        class ExampleBehaviour : public NativeBehaviour {
            A_OBJECT(ExampleBehaviour, NativeBehaviour, General)

            A_NOMETHODS()
            A_NOPROPERTIES()

        public:
            void start() {
                Log(Log::DBG) << "Start";
            }

            void update() {
                Log(Log::DBG) << "Update";
            }
        };
    \endcode
*/

NativeBehaviour::NativeBehaviour() :
        m_started(false) {
    Engine::addNativeBehaviour(this);
}

NativeBehaviour::~NativeBehaviour() {
    Engine::removeNativeBehaviour(this);
}

/*!
    Start is called on the same frame when a script is enabled just before the update method will be called the first time.
*/
void NativeBehaviour::start() {

}
/*!
    Update is called every frame, if the NativeBehaviour is enabled.
*/
void NativeBehaviour::update() {

}
/*!
    Returns true if the component is flagged as started; otherwise returns false.
    \note This method is used for internal purposes and shouldn't be called manually.

    \internal
*/
bool NativeBehaviour::isStarted() const {
    return m_started;
}
/*!
    Marks behaviour as \a started.
    \note This method is used for internal purposes and shouldn't be called manually.

    \internal
*/
void NativeBehaviour::setStarted(bool started) {
    m_started = started;
}
