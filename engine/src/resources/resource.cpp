#include "resources/resource.h"

#include "systems/resourcesystem.h"

#include <assert.h>

/*!
    \module Resources

    \title Thunder Engine Software Developer Kit

    \brief Contains base asset classes.
*/

/*!
    \class Resource
    \brief Base class for every resource in Thunder Engine.
    \inmodule Resources

    \note This class must be a superclass only and shouldn't be created manually.
*/

/*!
    \enum ResourceState

    Status for the resource.

    \value Invalid \c The state is invalid (Unable to load resource).
    \value Loading \c This resource is loading from the disc.
    \value ToBeUpdated \c This resource must be updated. Mostly used for the graphical resources to upload resouce to the graphical system. In case of resource doesn't require updating internal data. The resource must switch state to Ready immediately.
    \value Ready \c This resource is ready to use.
    \value Suspend \c This resource is not needed at this moment. In case of resource system will require additional memory suspended resources will be unloaded.
    \value Unloading \c This resource is marked to be unloaded. Mostly used for the graphical resources to unload resources from the graphical system. In case of resource doesn't require to unload internal data. The resource must switch state to ToBeDeleted immediately.
    \value ToBeDeleted \c This resource will be deleted soon. Resources with this state must not be used anywhere.
*/

Resource::Resource() :
        m_state(Resource::Invalid),
        m_last(Resource::Invalid),
        m_referenceCount(0) {

}

Resource::Resource(const Resource &origin) :
        m_observers(origin.m_observers),
        m_state(origin.m_state),
        m_last(origin.m_last),
        m_referenceCount(origin.m_referenceCount) {

}

Resource::~Resource() {
    assert(m_referenceCount == 0);

    ResourceSystem *system = Engine::resourceSystem();
    if(system) {
        system->deleteFromCahe(this);
    }
}
/*!
    Subscribes \a callback fro \a object to handle resource status.
    Increases reference count.
*/
void Resource::subscribe(ResourceUpdatedCallback callback, void *object) {
    unique_lock<mutex> locker(m_mutex);
    m_observers.push_back(make_pair(callback, object));
    locker.unlock();
    incRef();
}
/*!
    Unsubscribes an \a object to stop handle resource status.
    Decreases reference count.
*/
void Resource::unsubscribe(void *object) {
    unique_lock<mutex> locker(m_mutex);
    auto it = m_observers.begin();
    while(it != m_observers.end()) {
        if((it->second) == object) {
            it = m_observers.erase(it);
        } else {
            ++it;
        }
    }
    locker.unlock();
    decRef();
}
/*!
    Returns state for the resource.
    For possible states please see Resource::ResourceState.
*/
Resource::State Resource::state() const {
    return m_state;
}
/*!
    Switches the current state to a new \a state for the resource.
*/
void Resource::switchState(State state) {
    switch(state) {
        case ToBeUpdated: m_state = Ready; break;
        case Unloading: m_state = ToBeDeleted; break;
        default: m_state = state; break;
    }
    notifyCurrentState();
}
/*!
    Returns true in case of resource can be unloaded from GPU; otherwise returns false.
*/
bool Resource::isUnloadable() {
    return false;
}
/*!
    Sets new \a state for the resource.
*/
void Resource::setState(State state) {
    m_state = state;
    notifyCurrentState();
}
/*!
    Notifies subscribers about the current state of the resource.
*/
void Resource::notifyCurrentState() {
    unique_lock<mutex> locker(m_mutex);
    for(auto it : m_observers) {
        (*it.first)(m_state, it.second);
    }
}
/*!
    Increases the reference counter for the resource.
*/
void Resource::incRef() {
    if(m_referenceCount <= 0 && m_state == Suspend) {
        setState(m_last);
    }
    m_referenceCount++;
}
/*!
    Decreases the reference counter for the resource.
    In case of the reference count becomes zero the resource set to ResourceState::Suspend state.
*/
void Resource::decRef() {
    m_referenceCount--;

    assert(m_referenceCount >= 0);
    if(m_referenceCount <= 0 && m_state != Suspend) {
        m_last = m_state;
        setState(Suspend);
    }
}
