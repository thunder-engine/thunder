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
    m_started = true;
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
