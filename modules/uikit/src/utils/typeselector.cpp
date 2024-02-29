#include "utils/typeselector.h"

#include "components/widget.h"

TypeSelector::TypeSelector(const std::string &typeName) {
    m_typeName = typeName;
    m_selectorType = Selector::TypeSelector;
}

std::string TypeSelector::tagName() {
    return m_typeName;
}

bool TypeSelector::isMeet(Widget *widget) {
    return widget->typeName() == m_typeName;
}

bool TypeSelector::isBaseSelector() const {
    return true;
}

int TypeSelector::weight() {
    return 1;
}
