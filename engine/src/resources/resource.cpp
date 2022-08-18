#include "resources/resource.h"

#include <mutex>

#include "systems/resourcesystem.h"

class ResourcePrivate {
public:
    ResourcePrivate() :
        m_State(Resource::Invalid),
        m_Last(Resource::Invalid),
        m_ReferenceCount(0) {

    }
    Resource::ResourceState m_State;
    Resource::ResourceState m_Last;
    uint32_t m_ReferenceCount;
    list<Resource::IObserver *> m_Observers;
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
    \enum Resource::ResourceState

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
    Subscribes \a observer to handle resource status.
*/
void Resource::subscribe(IObserver *observer) {
    unique_lock<mutex> locker(p_ptr->m_Mutex);
    p_ptr->m_Observers.push_back(observer);
}
/*!
    Unsubscribes \a observer to stop handle resource status.
*/
void Resource::unsubscribe(IObserver *observer) {
    unique_lock<mutex> locker(p_ptr->m_Mutex);
    auto it = p_ptr->m_Observers.begin();
    while(it != p_ptr->m_Observers.end()) {
        if((*it) == observer) {
            it = p_ptr->m_Observers.erase(it);
        } else {
            ++it;
        }
    }
}
/*!
    Returns state for the resource.
    For possible states please see Resource::ResourceState.
*/
Resource::ResourceState Resource::state() const {
    return p_ptr->m_State;
}
/*!
    Switches the current state to a new \a state for the resource.
*/
void Resource::switchState(ResourceState state) {
    switch(state) {
        case ToBeUpdated: p_ptr->m_State = Ready; break;
        case Unloading: p_ptr->m_State = ToBeDeleted; break;
        default: p_ptr->m_State = state; break;
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
    p_ptr->m_State = state;
    notifyCurrentState();
}
/*!
    Notifies subscribers about the current state of the resource.
*/
void Resource::notifyCurrentState() {
    unique_lock<mutex> locker(p_ptr->m_Mutex);
    for(auto it : p_ptr->m_Observers) {
        it->resourceUpdated(this, p_ptr->m_State);
    }
}
/*!
    Increases the reference counter for the resource.
*/
void Resource::incRef() {
    if(p_ptr->m_ReferenceCount <= 0 && p_ptr->m_State == Suspend) {
        setState(p_ptr->m_Last);
    }
    p_ptr->m_ReferenceCount++;
}
/*!
    Decreases the reference counter for the resource.
    In case of the reference count becomes zero the resource set to ResourceState::Suspend state.
*/
void Resource::decRef() {
    p_ptr->m_ReferenceCount--;
    if(p_ptr->m_ReferenceCount <= 0 && p_ptr->m_State != Suspend) {
        p_ptr->m_Last = p_ptr->m_State;
        setState(Suspend);
    }
}
