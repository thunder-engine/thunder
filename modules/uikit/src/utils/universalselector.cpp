#include "utils/universalselector.h"

UniversalSelector::UniversalSelector() {
    m_selectorType = Selector::UniversalSelector;
}

bool UniversalSelector::isMeet(Widget *widget) {
    return true;
}

bool UniversalSelector::isBaseSelector() const {
    return true;
}

int UniversalSelector::weight() {
    return 0;
}
