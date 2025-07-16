#include "utils/idselector.h"

#include "components/widget.h"

IdSelector::IdSelector(const String &id) {
    m_id = id;
    m_selectorType = Selector::IDSelector;
}

bool IdSelector::isMeet(Widget *widget) {
    return m_id == widget->name();
}

bool IdSelector::isBaseSelector() const {
    return true;
}

int IdSelector::weight() {
    return 100;
}
