#include "utils/signselector.h"

SignSelector::SignSelector(SignType type) {
    m_signType = type;
    m_selectorType = Selector::SignSelector;
}

bool SignSelector::operator>(SignSelector *other) {
    SignType otherType = other->signType();
    return m_signType == Concat && otherType != Concat;
}

SignSelector::SignType SignSelector::signType() {
    return m_signType;
}

bool SignSelector::isMeet(Widget *) {
    return false;
}

bool SignSelector::isBaseSelector() const {
    return true;
}

int SignSelector::weight() {
    return 0;
}
