#include "resources/resource.h"

#include <mutex>

#include "systems/resourcesystem.h"

class ResourcePrivate {
public:
    ResourcePrivate() :
            m_state(ResourceState::Invalid),
            m_last(ResourceState::Invalid),
            m_referenceCount(0) {

    }
    list<pair<Resource::ResourceUpdatedCallback, void *>> m_observers;

    ResourceState m_state;
    ResourceState m_last;

    uint32_t m_referenceCount;

    mutex m_Mutex;

};

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
    p_ptr(new ResourcePrivate) {

}

Resource::~Resource() {
    ResourceSystem *system = Engine::resourceSystem();
    if(system) {
        system->deleteFromCahe(this);
    }
    delete p_ptr;
}
/*!
    Subscribes \a callback fro \a object to handle resource status.
*/
void Resource::subscribe(ResourceUpdatedCallback callback, void *object) {
    unique_lock<mutex> locker(p_ptr->m_Mutex);
    p_ptr->m_observers.push_back(make_pair(callback, object));
}
/*!
    Unsubscribes an \a object to stop handle resource status.
*/
void Resource::unsubscribe(void *object) {
    unique_lock<mutex> locker(p_ptr->m_Mutex);
    auto it = p_ptr->m_observers.begin();
    while(it != p_ptr->m_observers.end()) {
        if((it->second) == object) {
            it = p_ptr->m_observers.erase(it);
        } else {
            ++it;
        }
    }
}
/*!
    Returns state for the resource.
    For possible states please see Resource::ResourceState.
*/
ResourceState Resource::state() const {
    return p_ptr->m_state;
}
/*!
    Switches the current state to a new \a state for the resource.
*/
void Resource::switchState(ResourceState state) {
    switch(state) {
        case ToBeUpdated: p_ptr->m_state = Ready; break;
        case Unloading: p_ptr->m_state = ToBeDeleted; break;
        default: p_ptr->m_state = state; break;
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
void Resource::setState(ResourceState state) {
    p_ptr->m_state = state;
    notifyCurrentState();
}
/*!
    Notifies subscribers about the current state of the resource.
*/
void Resource::notifyCurrentState() {
    unique_lock<mutex> locker(p_ptr->m_Mutex);
    for(auto it : p_ptr->m_observers) {
        (*it.first)(p_ptr->m_state, it.second);
    }
}
/*!
    Increases the reference counter for the resource.
*/
void Resource::incRef() {
    if(p_ptr->m_referenceCount <= 0 && p_ptr->m_state == Suspend) {
        setState(p_ptr->m_last);
    }
    p_ptr->m_referenceCount++;
}
/*!
    Decreases the reference counter for the resource.
    In case of the reference count becomes zero the resource set to ResourceState::Suspend state.
*/
void Resource::decRef() {
    p_ptr->m_referenceCount--;
    if(p_ptr->m_referenceCount <= 0 && p_ptr->m_state != Suspend) {
        p_ptr->m_last = p_ptr->m_state;
        setState(Suspend);
    }
}
