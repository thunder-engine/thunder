#include "utils/typeselector.h"

#include "components/widget.h"

TypeSelector::TypeSelector(const TString &typeName) {
    m_typeName = typeName;
    m_selectorType = Selector::TypeSelector;
}

TString TypeSelector::tagName() {
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
