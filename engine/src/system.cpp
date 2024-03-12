#include "system.h"

/*!
    \class System
    \brief A base interface for all in-game systems.
    \inmodule Engine

    Systems are a basic processors for each Component in the game.
    \note All methods will be called internaly in the engine.
    \note Systems can process only components which registered in this system.
    \note Systems can be executed one by one or in parallel based on thread policy.
*/

/*!
    \enum System::ThreadPolicy

    \value Main \c The System::update will be executed one by one in the main thread. This method is handy when you need to execute systems with exact sequence. This policy uses only one CPU core.
    \value Pool \c The System::update will be executed in the dedicated thread pool. Please note, there is no warranty of a sequence of execution for this case. This policy is preferable because it utilizes CPU cores more efficiently.
*/

System::System() :
    m_world(nullptr) {

}
/*!
    Can be used to initialize and execute necessary routines.
    This method will be called automatically just after the engine started.
    Returns true if success.
*/
bool System::init() {
    return true;
}
/*!
    Can be used to reset all internal system states.
    This method will be called automatically just after the engine started.
*/
void System::reset() {

}
/*!
    All processing operations for the current \a world must be done in this method.
*/
void System::update(World *world) {

}
/*!
    Returns the thread policy of the system.
    For more details please refer to System::ThreadPolicy enum.
*/
int System::threadPolicy() const {
    return 0;
}
/*!
    This method is a callback to react on saving game settings.
*/
void System::syncSettings() const {

}
/*!
    This method is a helper to initialize specifically a new \a component.
    Usually used in the editor.
*/
void System::composeComponent(Component *component) const {
    A_UNUSED(component);
}
/*!
    Sets active \a world.
*/
void System::setActiveWorld(World *world) {
    m_world = world;
}
/*!
    Processes all incoming events and executes the System::update method.
*/
void System::processEvents() {
    ObjectSystem::processEvents();

    update(m_world);
}
