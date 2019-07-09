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

Resource::Resource() :
    p_ptr(new ResourcePrivate) {

}

Resource::~Resource() {
    delete p_ptr;
}

Resource::ResourceState Resource::state() const {
    return p_ptr->m_State;
}

void Resource::setState(ResourceState state) {
    p_ptr->m_State = state;
}

void Resource::incRef() {
    if(p_ptr->m_ReferenceCount <= 0 && p_ptr->m_State == Suspend) {
        p_ptr->m_State = p_ptr->m_Last;
    }
    p_ptr->m_ReferenceCount++;
}

void Resource::decRef() {
    p_ptr->m_ReferenceCount--;
    if(p_ptr->m_ReferenceCount <= 0 && p_ptr->m_State != Suspend) {
        p_ptr->m_Last = p_ptr->m_State;
        p_ptr->m_State = Suspend;
    }
}
