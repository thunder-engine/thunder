#include "resources/resource.h"

class ResourcePrivate {
public:
    ResourcePrivate() :
        m_Valid(false) {

    }
    bool m_Valid;
};

Resource::Resource() :
    p_ptr(new ResourcePrivate) {

}

Resource::~Resource() {
    delete p_ptr;
}

bool Resource::isValid() {
    return p_ptr->m_Valid;
}

void Resource::setValid(bool valid) {
    p_ptr->m_Valid = valid;
}
