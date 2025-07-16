#include "utils/typeselector.h"

#include "components/widget.h"

TypeSelector::TypeSelector(const String &typeName) {
    m_typeName = typeName;
    m_selectorType = Selector::TypeSelector;
}

String TypeSelector::tagName() {
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
