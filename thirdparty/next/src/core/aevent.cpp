#include "core/aevent.h"

AEvent::AEvent(Type type) {
    PROFILE_FUNCTION()
    m_Type  = type;
}

AEvent::Type AEvent::type() const {
    PROFILE_FUNCTION()
    return m_Type;
}
