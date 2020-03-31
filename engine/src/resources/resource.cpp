#include "resources/resource.h"

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
};

/*!
    \module Resource

    \title Thunder Engine Software Developer Kit

    \brief Contains base asset classes.
*/

/*!
    \class Resource
    \brief Base class for every resource in Thunder Engine.
    \inmodule Resource

    \note This class must be a superclass only and shouldn't be created manually.
*/

/*!
    \enum Resource::ResourceState

    Status for the resource.

    \value Invalid \c The state is invalid.
    \value Loading \c This resource is loading.
    \value ToBeUpdated \c This resource must be updated. Mostly used for the graphical rendering to upload textures and meshes to the graphical system.
    \value Ready \c This resource is ready to use.
    \value Suspend \c This resource is not needed at this moment. In case of resource system will require additional memory suspended resources will be unloaded.
    \value ToBeDeleted \c This resource will be unloaded soon. Resources with this state shouldn't be used anywhere.
*/

Resource::Resource() :
    p_ptr(new ResourcePrivate) {

}

Resource::~Resource() {
    delete p_ptr;
}
/*!
    Returns state for the resource.
    For possible states please see Resource::ResourceState.
*/
Resource::ResourceState Resource::state() const {
    return p_ptr->m_State;
}
/*!
    Sets new \a state for the resource.
*/
void Resource::setState(ResourceState state) {
    p_ptr->m_State = state;
}
/*!
    Increases the reference counter for the resource.
*/
void Resource::incRef() {
    if(p_ptr->m_ReferenceCount <= 0 && p_ptr->m_State == Suspend) {
        p_ptr->m_State = p_ptr->m_Last;
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
        p_ptr->m_State = Suspend;
    }
}
