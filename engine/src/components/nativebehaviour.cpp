#include "components/nativebehaviour.h"

/*!
    \class NativeBehaviour
    \brief Base class for all programmable game logic.
    \inmodule Components

    All programmable game logic must be derived from NativeBehaviour class.

    Example:
    \code
        class ExampleBehaviour : public NativeBehaviour {
            A_REGISTER(ExampleBehaviour, NativeBehaviour, General)

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

NativeBehaviour::NativeBehaviour() {

}

NativeBehaviour::~NativeBehaviour() {
    static_cast<Engine *>(system())->removeNativeBehaviour(this);
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
    \internal
*/
void NativeBehaviour::setSystem(ObjectSystem *system) {
    Object::setSystem(system);

    Engine *engine = static_cast<Engine *>(system);
    engine->addNativeBehaviour(this);
}
