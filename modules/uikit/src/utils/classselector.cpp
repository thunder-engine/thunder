#include "utils/classselector.h"

#include "components/widget.h"

#include <algorithm>

ClassSelector::ClassSelector(const std::string &cls) {
    m_class = cls;
    m_selectorType = Selector::ClassSelector;
}

bool ClassSelector::isMeet(Widget *widget) {
    auto classes = widget->classes();
    return std::find(classes.begin(), classes.end(), m_class) != classes.end();
}

bool ClassSelector::isBaseSelector() const {
    return true;
}

int ClassSelector::weight() {
    return 10;
}
