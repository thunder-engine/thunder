#include "core/event.h"

Event::Event(Type type) {
    PROFILE_FUNCTION()
    m_Type  = type;
}

Event::Type Event::type() const {
    PROFILE_FUNCTION()
    return m_Type;
}
