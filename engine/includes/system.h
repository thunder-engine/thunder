#ifndef ABSTRACTSYSTEM_H
#define ABSTRACTSYSTEM_H

#include <string>
#include <stdint.h>

class Engine;
class Scene;
class IController;

using namespace std;

class ISystem {
public:
    /*!
        Constructor of System class.
        @param[in]  engine      Pointer to Engine object.
    */
    ISystem                     (Engine *engine) : m_pEngine(engine) { }
    virtual ~ISystem            () {}

    /*!
        Initialization of system.
        @return true            Intialization successful.
    */
    virtual bool                init                        () = 0;

    virtual const char         *name                        () const = 0;
    /*!
        Tha main procedure procedure that is called every cycle.
        @param[in]  scene       Reference to scene.
        @param[in]  resource    Additional value which can be used for external purporces.
    */
    virtual void                update                      (Scene &scene, uint32_t resource = 0) = 0;
    /*!
        Developers is able to override controller for this particular system.
        @param[in]  controller  Pointer to controller (can be NULL).
    */
    virtual void                overrideController          (IController *controller) = 0;

protected:
    Engine                     *m_pEngine;
};

#endif // ABSTRACTSYSTEM_H
