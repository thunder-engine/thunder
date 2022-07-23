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

/*!
    \fn bool System::init()

    Can be used to initialize and execute necessary routines.
    This method will be called automatically just after the engine started.
    Returns true if success.
*/

/*!
    \fn void System::update(Scene *scene)

    All processing operations for the current \a scene must be done in this method.
*/

/*!
    \fn char *System::name() const

    Returns the name of system.
*/

/*!
    \fn int System::threadPolicy() const

    Returns the thread policy of the system.
    For more details please refer to System::ThreadPolicy enum.
*/

System::System() :
    m_pSceneGraph(nullptr) {

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
    Sets active \a scene.
*/
void System::setActiveScene(SceneGraph *sceneGraph) {
    m_pSceneGraph = sceneGraph;
}
/*!
    Processes all incoming events and executes the System::update method.
*/
void System::processEvents() {
    ObjectSystem::processEvents();

    update(m_pSceneGraph);
}
