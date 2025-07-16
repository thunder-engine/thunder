#include "utils/pseudoselector.h"

PseudoSelector::PseudoSelector(const String &data) {
    m_selectorType = Selector::PseudoSelector;
    m_data = data;
    m_parameter = NULL;
}

PseudoSelector::~PseudoSelector() {
    delete m_parameter;
    m_parameter = NULL;
}

bool PseudoSelector::isMeet(Widget *) {
    return false;
}

bool PseudoSelector::isBaseSelector() const {
    return true;
}

int PseudoSelector::weight() {
    return 10;
}

void PseudoSelector::setParameter(PseudoSelector::Parameter *p) {
    if(m_parameter == p) {
        return;
    }

    if(m_parameter) {
        delete m_parameter;
    }
    m_parameter = p;
}

PseudoSelector::Parameter* PseudoSelector::parameter() {
    return m_parameter;
}
